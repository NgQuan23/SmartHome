#include "storage.h"
#include <LittleFS.h>
#include "config.h"
#include "wifi_mqtt.h"
#include "firebase_client.h"
#include <WiFi.h>

static const char *STORAGE_PATH = "/events.log";
static const char *STORAGE_TMP_PATH = "/events.tmp";
static bool storageMounted = false;

static bool mountStorage(bool formatOnFail){
  if (!LittleFS.begin(formatOnFail)) {
    return false;
  }

  if (LittleFS.totalBytes() != 0) {
    return true;
  }

  Serial.println("LittleFS mounted with zero-sized partition, formatting...");
  LittleFS.end();
  if (!LittleFS.format()) {
    return false;
  }
  return LittleFS.begin(false);
}

void storageInit(){
  storageMounted = mountStorage(true);
  if (!storageMounted){
    Serial.println("LittleFS mount failed");
    return;
  }

  Serial.printf(
    "LittleFS mounted: used=%u bytes, total=%u bytes\n",
    static_cast<unsigned>(LittleFS.usedBytes()),
    static_cast<unsigned>(LittleFS.totalBytes())
  );
}

bool storageAvailable(){
  return storageMounted;
}

bool storageAppend(const String &jsonLine){
  if (!storageMounted) {
    Serial.println("Storage append skipped: LittleFS unavailable");
    return false;
  }

  File f = LittleFS.open(STORAGE_PATH, FILE_APPEND);
  if (!f){
    Serial.println("Failed to open storage file for append");
    return false;
  }

  size_t written = f.println(jsonLine);
  f.close();
  if (written == 0) {
    Serial.println("Failed to write queued event to LittleFS");
    return false;
  }
  return true;
}

void storageFlush(){
  if (!storageMounted) {
    Serial.println("storageFlush: LittleFS unavailable, skipping");
    return;
  }
  if (WiFi.status() != WL_CONNECTED){
    Serial.println("storageFlush: WiFi not connected, skipping");
    return;
  }
  if (!LittleFS.exists(STORAGE_PATH)) return;

  File f = LittleFS.open(STORAGE_PATH, FILE_READ);
  if (!f) return;

  LittleFS.remove(STORAGE_TMP_PATH);
  File tmp = LittleFS.open(STORAGE_TMP_PATH, FILE_WRITE);
  if (!tmp) {
    Serial.println("storageFlush: failed to open temp queue file");
    f.close();
    return;
  }

  Serial.println("Flushing stored events...");
  size_t flushedCount = 0;
  size_t keptCount = 0;
  while (f.available()){
    String line = f.readStringUntil('\n');
    line.trim();
    if (line.length() == 0) continue;

    bool mqttOk = mqttPublish(TELEMETRY_TOPIC, line.c_str());
    bool firebaseOk = firebasePushTelemetryPayload(line, false);
    if (mqttOk && firebaseOk) {
      ++flushedCount;
      delay(50);
      continue;
    }

    tmp.println(line);
    ++keptCount;
    delay(50);
  }

  f.close();
  tmp.close();

  LittleFS.remove(STORAGE_PATH);
  if (keptCount == 0) {
    LittleFS.remove(STORAGE_TMP_PATH);
  } else if (!LittleFS.rename(STORAGE_TMP_PATH, STORAGE_PATH)) {
    Serial.println("storageFlush: failed to rotate temp queue file");
  }

  Serial.printf("storageFlush: flushed=%u, kept=%u\n", static_cast<unsigned>(flushedCount), static_cast<unsigned>(keptCount));
}
