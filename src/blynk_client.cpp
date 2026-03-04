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
  Blynk.config(BLYNK_AUTH_TOKEN);
  Blynk.connect();
  Serial.println("Blynk initialized");
}

void blynkPublishTelemetry(int gas, float distance, bool motion, int distanceLevel){

  Blynk.virtualWrite(V0, gas);
  if (distance < 0) Blynk.virtualWrite(V1, "--");
  else Blynk.virtualWrite(V1, (int)distance);
  Blynk.virtualWrite(V2, motion ? 1 : 0);
  Blynk.virtualWrite(V4, distanceLevel);
}

void blynkNotify(const char* msg){

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
