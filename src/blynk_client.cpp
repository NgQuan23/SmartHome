#include "blynk_client.h"
#include "config.h"
#if ENABLE_BLYNK
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>

void blynkInit(){
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Blynk: WiFi not connected, skipping Blynk init");
    return;
  }
  Blynk.begin(BLYNK_AUTH, WIFI_SSID, WIFI_PASS);
  Serial.println("Blynk initialized");
}

void blynkPublishTelemetry(int gas, float distance, bool motion, int distanceLevel){
  // Map telemetry to virtual pins
  Blynk.virtualWrite(V0, gas);
  if (distance < 0) Blynk.virtualWrite(V1, "--");
  else Blynk.virtualWrite(V1, (int)distance);
  Blynk.virtualWrite(V2, motion ? 1 : 0);
  Blynk.virtualWrite(V4, distanceLevel);
}

void blynkNotify(const char* msg){
  // Some Blynk library variants don't expose notify(); send message to a virtual pin (V3)
  Blynk.virtualWrite(V3, msg);
}

void blynkRun(){
  Blynk.run();
}

#else
void blynkInit(){}
void blynkPublishTelemetry(int gas, float distance, bool motion){}
void blynkNotify(const char* msg){}
void blynkRun(){}
#endif
