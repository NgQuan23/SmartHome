#include "firebase_client.h"
#include "config.h"
#if ENABLE_FIREBASE
#include <Firebase_ESP_Client.h>
#include <WiFi.h>
#include <ArduinoJson.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/task.h>
#include "storage.h"

namespace {

constexpr size_t FIREBASE_QUEUE_LENGTH = 12;
constexpr size_t FIREBASE_PAYLOAD_MAX_LEN = 224;
constexpr uint8_t FIREBASE_MAX_RETRIES = 3;
constexpr unsigned long FIREBASE_LOG_INTERVAL_MS = 15000;
constexpr unsigned long FIREBASE_STATUS_INTERVAL_MS = 30000;
constexpr TickType_t FIREBASE_RETRY_DELAY = pdMS_TO_TICKS(1000);
constexpr TickType_t FIREBASE_IDLE_DELAY = pdMS_TO_TICKS(200);

struct FirebaseTelemetryItem {
  char payload[FIREBASE_PAYLOAD_MAX_LEN];
  bool persistOnFailure;
};

FirebaseAuth auth;
FirebaseConfig configF;
FirebaseData fbdo;
FirebaseData fbdoCmd;
bool firebaseEnabled = false;
bool firebaseUrlDerived = false;
bool firebaseDisabledLogged = false;
unsigned long lastFirebaseTokenLogMs = 0;
int lastFirebaseTokenStatus = -1;
int lastFirebaseTokenErrorCode = 0;
unsigned long lastFirebasePushLogMs = 0;
String lastFirebasePushError;
const char *kTelemetryPath = "/devices/device1/telemetry";
const char *kStatusPath = "/devices/device1/status";
const char *kAwayModePath = "/devices/device1/switches/away_mode";
const char *kDistanceCriticalPath = "/devices/device1/settings/distance_critical";
const char *kGasWarningPath = "/devices/device1/settings/gas_warning";
QueueHandle_t firebaseQueue = nullptr;
TaskHandle_t firebaseTaskHandle = nullptr;
unsigned long lastFirebaseStatusWriteMs = 0;
volatile bool currentAwayMode = false;
volatile int currentDistanceCritical = 0;
volatile int currentGasWarning = 0;

String buildTelemetryPayload(int gas, float distance, bool motion){
  StaticJsonDocument<192> doc;
  doc["gas"] = gas;
  doc["distance"] = distance;
  doc["motion"] = motion ? 1 : 0;

  String payload;
  serializeJson(doc, payload);
  return payload;
}

String resolveFirebaseDatabaseUrl(){
  String url = String(FIREBASE_DATABASE_URL);
  url.trim();

  if (url.indexOf("console.firebase.google.com") >= 0) {
    String projectId = String(FIREBASE_PROJECT_ID);
    projectId.trim();
    if (projectId.length() == 0) {
      return "";
    }

    firebaseUrlDerived = true;
    url = "https://";
    url += projectId;
    url += "-default-rtdb.firebaseio.com/";
  }

  if (url.length() == 0) {
    return "";
  }

  if (!url.startsWith("http://") && !url.startsWith("https://")) {
    String temp = "https://";
    temp += url;
    url = temp;
  }
  if (!url.endsWith("/")) {
    url += "/";
  }

  return url;
}

bool hasFirebaseCredentials(){
  return (String(FIREBASE_USER_EMAIL).length() > 0 && String(FIREBASE_USER_PASSWORD).length() > 0) ||
        String(FIREBASE_DATABASE_SECRET).length() > 0;
}

const char *firebaseTokenTypeName(const TokenInfo &info)
{
  switch (info.type)
  {
  case token_type_undefined:
    return "undefined";
  case token_type_legacy_token:
    return "legacy token";
  case token_type_id_token:
    return "id token";
  case token_type_custom_token:
    return "custom token";
  case token_type_oauth2_access_token:
    return "oauth2";
  default:
    return "unknown";
  }
}

const char *firebaseTokenStatusName(const TokenInfo &info)
{
  switch (info.status)
  {
  case token_status_uninitialized:
    return "uninitialized";
  case token_status_on_initialize:
    return "initializing";
  case token_status_on_signing:
    return "signing";
  case token_status_on_request:
    return "request";
  case token_status_on_refresh:
    return "refresh";
  case token_status_ready:
    return "ready";
  case token_status_error:
    return "error";
  default:
    return "unknown";
  }
}

void firebaseTokenStatusCallback(TokenInfo info)
{
  const unsigned long now = millis();
  const bool statusChanged = info.status != lastFirebaseTokenStatus;
  const bool errorChanged = info.status == token_status_error && info.error.code != lastFirebaseTokenErrorCode;
  const bool intervalElapsed = now - lastFirebaseTokenLogMs >= FIREBASE_LOG_INTERVAL_MS;

  bool shouldLog = false;
  if (info.status == token_status_error) {
    shouldLog = errorChanged || intervalElapsed;
  } else if (info.status == token_status_ready) {
    shouldLog = statusChanged || intervalElapsed;
  }

  lastFirebaseTokenStatus = info.status;
  if (info.status == token_status_error) {
    lastFirebaseTokenErrorCode = info.error.code;
  }
  if (!shouldLog) {
    return;
  }

  lastFirebaseTokenLogMs = now;
  Serial.printf("Firebase token: type=%s status=%s", firebaseTokenTypeName(info), firebaseTokenStatusName(info));
  if (info.status == token_status_error) {
    Serial.printf(" code=%d msg=%s", info.error.code, info.error.message.c_str());
  }
  Serial.println();
}

bool buildFirebaseJson(const char *payload, FirebaseJson &json){
  StaticJsonDocument<256> doc;
  DeserializationError err = deserializeJson(doc, payload);
  if (err) {
    Serial.printf("Firebase payload parse failed: %s\n", err.c_str());
    return false;
  }

  size_t fieldCount = 0;
  if (!doc["gas"].isNull()) {
    json.set("gas", doc["gas"].as<int>());
    ++fieldCount;
  }
  if (!doc["distance"].isNull()) {
    json.set("distance", doc["distance"].as<float>());
    ++fieldCount;
  }
  if (!doc["distance_level"].isNull()) {
    json.set("distance_level", doc["distance_level"].as<int>());
    ++fieldCount;
  }
  if (!doc["motion"].isNull()) {
    json.set("motion", doc["motion"].as<int>());
    ++fieldCount;
  }

  return fieldCount > 0;
}

bool persistPayload(const char *payload, bool persistOnFailure) {
  if (!persistOnFailure) {
    return false;
  }
  return storageAppend(String(payload));
}

void updateConnectionStatus(bool forceWrite = false) {
  const unsigned long now = millis();
  if (!forceWrite && now - lastFirebaseStatusWriteMs < FIREBASE_STATUS_INTERVAL_MS) {
    return;
  }

  FirebaseJson statusJson;
  statusJson.set("connected", true);
  statusJson.set("ip", WiFi.localIP().toString());
  statusJson.set("rssi", WiFi.RSSI());
  statusJson.set("uptime_ms", static_cast<int>(now));

  if (Firebase.RTDB.setJSON(&fbdo, kStatusPath, &statusJson)) {
    lastFirebaseStatusWriteMs = now;
    Serial.println("Firebase: status heartbeat updated");
    return;
  }

  Serial.printf("Firebase status update failed: %s\n", fbdo.errorReason().c_str());
}

bool pushTelemetryNow(const char *payload){
  if (!firebaseEnabled) {
    if (!firebaseDisabledLogged) {
      firebaseDisabledLogged = true;
      Serial.println("Firebase task idle: configuration is incomplete");
    }
    return true;
  }
  if (WiFi.status() != WL_CONNECTED) {
    return false;
  }
  if (!Firebase.ready()) {
    return false;
  }

  FirebaseJson json;
  if (!buildFirebaseJson(payload, json)) {
    return true;
  }

  if (Firebase.RTDB.setJSON(&fbdo, kTelemetryPath, &json)){
    Serial.println("Firebase: telemetry updated");
    lastFirebasePushError = "";
    updateConnectionStatus(lastFirebaseStatusWriteMs == 0);
    return true;
  }

  const String error = fbdo.errorReason();
  const unsigned long now = millis();
  if (error != lastFirebasePushError || now - lastFirebasePushLogMs >= FIREBASE_LOG_INTERVAL_MS) {
    lastFirebasePushLogMs = now;
    lastFirebasePushError = error;
    Serial.printf("Firebase push failed: %s\n", error.c_str());
  }
  return false;
}

void processFirebaseItem(const FirebaseTelemetryItem &item) {
  for (uint8_t attempt = 0; attempt < FIREBASE_MAX_RETRIES; ++attempt) {
    if (pushTelemetryNow(item.payload)) {
      return;
    }
    vTaskDelay(FIREBASE_RETRY_DELAY);
  }

  if (item.persistOnFailure) {
    if (!persistPayload(item.payload, true)) {
      Serial.println("Firebase: failed to persist undelivered payload");
    }
  }
}

void firebaseTask(void*){
  FirebaseTelemetryItem item = {};
  unsigned long lastPoll = 0;

  for (;;) {
    if (!firebaseQueue) {
      vTaskDelay(FIREBASE_IDLE_DELAY);
      continue;
    }

    if (firebaseIsReady()) {
      unsigned long now = millis();
      if (now - lastPoll >= 2000) {
        lastPoll = now;
        bool mode = false;
        if (Firebase.RTDB.getBool(&fbdoCmd, kAwayModePath, &mode)) {
          if (currentAwayMode != mode) {
             currentAwayMode = mode;
             Serial.printf("Firebase: AwayMode updated to %s\n", mode ? "ON" : "OFF");
          }
        }
        
        int distCriticalVal = 0;
        if (Firebase.RTDB.getInt(&fbdoCmd, kDistanceCriticalPath, &distCriticalVal)) {
          if (currentDistanceCritical != distCriticalVal) {
             currentDistanceCritical = distCriticalVal;
             Serial.printf("Firebase: DistanceCritical updated to %d\n", distCriticalVal);
          }
        }
        
        int gasWarningVal = 0;
        if (Firebase.RTDB.getInt(&fbdoCmd, kGasWarningPath, &gasWarningVal)) {
          if (currentGasWarning != gasWarningVal) {
             currentGasWarning = gasWarningVal;
             Serial.printf("Firebase: GasWarning updated to %d\n", gasWarningVal);
          }
        }
      }
    }

    if (xQueueReceive(firebaseQueue, &item, FIREBASE_IDLE_DELAY) == pdPASS) {
      processFirebaseItem(item);
    }
  }
}

bool enqueuePayload(const String &payload, bool persistOnFailure){
  if (!firebaseEnabled) {
    if (!firebaseDisabledLogged) {
      firebaseDisabledLogged = true;
      Serial.println("Firebase disabled: telemetry will skip Firebase path");
    }
    return true;
  }

  if (!firebaseQueue) {
    Serial.println("Firebase: queue unavailable");
    return persistPayload(payload.c_str(), persistOnFailure);
  }

  FirebaseTelemetryItem item = {};
  snprintf(item.payload, sizeof(item.payload), "%s", payload.c_str());
  item.persistOnFailure = persistOnFailure;

  if (xQueueSend(firebaseQueue, &item, 0) == pdPASS) {
    return true;
  }

  Serial.println("Firebase: queue full, persisting payload");
  return persistPayload(item.payload, persistOnFailure);
}

}

void firebaseInit(){
  String databaseUrl = resolveFirebaseDatabaseUrl();
  firebaseEnabled = String(FIREBASE_API_KEY).length() > 0 && databaseUrl.length() > 0 && hasFirebaseCredentials();
  if (!firebaseEnabled) {
    Serial.println("Firebase disabled: missing API key, RTDB URL, or auth credentials");
    return;
  }

  configF.api_key = FIREBASE_API_KEY;
  configF.database_url = databaseUrl;
  configF.token_status_callback = firebaseTokenStatusCallback;
  configF.timeout.serverResponse = 10 * 1000;

  if (String(FIREBASE_USER_EMAIL).length() > 0 && String(FIREBASE_USER_PASSWORD).length() > 0) {
    auth.user.email = FIREBASE_USER_EMAIL;
    auth.user.password = FIREBASE_USER_PASSWORD;
  } else {
    configF.signer.tokens.legacy_token = FIREBASE_DATABASE_SECRET;
  }

  Firebase.reconnectNetwork(true);
  fbdo.setBSSLBufferSize(4096, 1024);
  fbdo.setResponseSize(2048);
  fbdoCmd.setBSSLBufferSize(2048, 1024);
  fbdoCmd.setResponseSize(1024);

  Firebase.begin(&configF, &auth);
  Firebase.setDoubleDigits(5);

  if (firebaseUrlDerived) {
    Serial.printf("Firebase RTDB URL inferred from project id: %s\n", databaseUrl.c_str());
  }
  Serial.println("Firebase initialized");

  if (!firebaseQueue) {
    firebaseQueue = xQueueCreate(FIREBASE_QUEUE_LENGTH, sizeof(FirebaseTelemetryItem));
  }
  if (!firebaseQueue) {
    Serial.println("Firebase: failed to create worker queue");
    return;
  }

  if (!firebaseTaskHandle) {
    BaseType_t created = xTaskCreatePinnedToCore(
      firebaseTask,
      "firebaseTask",
      7168,
      nullptr,
      1,
      &firebaseTaskHandle,
      1
    );
    if (created != pdPASS) {
      firebaseTaskHandle = nullptr;
      Serial.println("Firebase: failed to start worker task");
    }
  }
}

bool firebasePushTelemetryPayload(const String &payload, bool queueOnFailure){
  return enqueuePayload(payload, queueOnFailure);
}

bool firebasePushTelemetry(int gas, float distance, bool motion, bool queueOnFailure){
  return firebasePushTelemetryPayload(buildTelemetryPayload(gas, distance, motion), queueOnFailure);
}

bool firebaseIsReady(){
  return firebaseEnabled && WiFi.status() == WL_CONNECTED && Firebase.ready();
}
bool firebaseGetAwayMode(){
  return currentAwayMode;
}
int firebaseGetDistanceCritical(){
  return currentDistanceCritical;
}
int firebaseGetGasWarning(){
  return currentGasWarning;
}
#else
void firebaseInit(){ }
bool firebasePushTelemetry(int gas, float distance, bool motion, bool queueOnFailure){ return false; }
bool firebasePushTelemetryPayload(const String &payload, bool queueOnFailure){ return false; }
bool firebaseIsReady(){ return false; }
bool firebaseGetAwayMode(){ return false; }
int firebaseGetDistanceCritical(){ return 0; }
int firebaseGetGasWarning(){ return 0; }
#endif
