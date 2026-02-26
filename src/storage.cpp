#include "storage.h"
#include <LittleFS.h>
#include "config.h"
#include "wifi_mqtt.h"
#include "firebase_client.h"
#include <WiFi.h>

static const char *STORAGE_PATH = "/events.log";

void storageInit(){
  if (!LittleFS.begin(true)){
    Serial.println("LittleFS mount failed");
  }
}

void storageAppend(const String &jsonLine){
  File f = LittleFS.open(STORAGE_PATH, FILE_APPEND);
  if (!f){
    Serial.println("Failed to open storage file for append");
    return;
  }
  f.println(jsonLine);
  f.close();
}

void storageFlush(){
  if (WiFi.status() != WL_CONNECTED){
    Serial.println("storageFlush: WiFi not connected, skipping");
    return;
  }
  if (!LittleFS.exists(STORAGE_PATH)) return;
  File f = LittleFS.open(STORAGE_PATH, FILE_READ);
  if (!f) return;
  Serial.println("Flushing stored events...");
  while (f.available()){
    String line = f.readStringUntil('\n');
    if (line.length() == 0) continue;
    firebasePushTelemetry(-1, -1, false);
    mqttPublish(TELEMETRY_TOPIC, line.c_str());
    delay(50);
  }
  f.close();
  LittleFS.remove(STORAGE_PATH);
}
