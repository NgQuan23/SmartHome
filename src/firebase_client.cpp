#include "firebase_client.h"
#include "config.h"
#if ENABLE_FIREBASE
#include <Firebase_ESP_Client.h>
#include <addons/TokenHelper.h>
#include <WiFi.h>
#include <ArduinoJson.h>
#include "storage.h"

namespace {

FirebaseAuth auth;
FirebaseConfig configF;
FirebaseData fbdo;
bool firebaseEnabled = false;
bool firebaseUrlDerived = false;
const char *kTelemetryPath = "/devices/device1/telemetry";

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

bool buildFirebaseJson(const String &payload, FirebaseJson &json){
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

bool queuePayload(const String &payload, bool queueOnFailure){
  if (!queueOnFailure) {
    return false;
  }
  return storageAppend(payload);
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
  configF.token_status_callback = tokenStatusCallback;
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
  Firebase.begin(&configF, &auth);
  Firebase.setDoubleDigits(5);

  if (firebaseUrlDerived) {
    Serial.printf("Firebase RTDB URL inferred from project id: %s\n", databaseUrl.c_str());
  }
  Serial.println("Firebase initialized");
}

bool firebasePushTelemetryPayload(const String &payload, bool queueOnFailure){
  if (!firebaseEnabled) {
    Serial.println("Firebase skipped: configuration is incomplete");
    return false;
  }
  if (WiFi.status() != WL_CONNECTED){
    return queuePayload(payload, queueOnFailure);
  }
  if (!Firebase.ready()) {
    Serial.println("Firebase not ready yet, keeping telemetry queued");
    return queuePayload(payload, queueOnFailure);
  }

  FirebaseJson json;
  if (!buildFirebaseJson(payload, json)) {
    return false;
  }

  if (Firebase.RTDB.pushJSON(&fbdo, kTelemetryPath, &json)){
    Serial.println("Firebase: telemetry pushed");
    return true;
  }

  Serial.printf("Firebase push failed: %s\n", fbdo.errorReason().c_str());
  queuePayload(payload, queueOnFailure);
  return false;
}

bool firebasePushTelemetry(int gas, float distance, bool motion, bool queueOnFailure){
  return firebasePushTelemetryPayload(buildTelemetryPayload(gas, distance, motion), queueOnFailure);
}

bool firebaseIsReady(){
  return firebaseEnabled && WiFi.status() == WL_CONNECTED && Firebase.ready();
}
#else
void firebaseInit(){ }
bool firebasePushTelemetry(int gas, float distance, bool motion, bool queueOnFailure){ return false; }
bool firebasePushTelemetryPayload(const String &payload, bool queueOnFailure){ return false; }
bool firebaseIsReady(){ return false; }
#endif
