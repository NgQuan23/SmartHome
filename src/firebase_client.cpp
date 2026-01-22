#include "firebase_client.h"
#include "config.h"
#if ENABLE_FIREBASE
#include <Firebase_ESP_Client.h>
#include <addons/TokenHelper.h>
#include <WiFi.h>
#include <ArduinoJson.h>
#include "storage.h"

// Provide the token generation process info.
FirebaseAuth auth;
FirebaseConfig configF;
FirebaseData fbdo;

void firebaseInit(){
  configF.api_key = FIREBASE_API_KEY;
  configF.database_url = FIREBASE_HOST;
  // Optional: set callback for long running token generation
  Firebase.begin(&configF, &auth);
  Firebase.reconnectWiFi(true);
  Serial.println("Firebase initialized");
}

void firebasePushTelemetry(int gas, float distance, bool motion){
  if (WiFi.status() != WL_CONNECTED){
    // store payload for later sync
    StaticJsonDocument<192> doc;
    doc["gas"] = gas;
    doc["distance"] = distance;
    doc["motion"] = motion ? 1 : 0;
    char buf[192];
    serializeJson(doc, buf);
    storageAppend(String(buf));
    return;
  }
  FirebaseJson json;
  json.set("gas", gas);
  json.set("distance", distance);
  json.set("motion", motion ? 1 : 0);
  String path = "/devices/device1/telemetry.json";
  if (Firebase.RTDB.pushJSON(&fbdo, "/devices/device1/telemetry", &json)){
    Serial.println("Firebase: telemetry pushed");
  } else {
    Serial.printf("Firebase push failed: %s\n", fbdo.errorReason().c_str());
    // fallback store
    StaticJsonDocument<192> doc;
    doc["gas"] = gas;
    doc["distance"] = distance;
    doc["motion"] = motion ? 1 : 0;
    char buf[192];
    serializeJson(doc, buf);
    storageAppend(String(buf));
  }
}
#else
void firebaseInit(){ }
void firebasePushTelemetry(int gas, float distance, bool motion){ }
#endif
