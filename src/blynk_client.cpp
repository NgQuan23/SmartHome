#include "blynk_client.h"
#include "config.h"
#if ENABLE_BLYNK
#define BLYNK_PRINT Serial
#define BLYNK_DEBUG
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>

void blynkInit(){
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Blynk: WiFi not connected, skipping Blynk init");
    return;
  }
  Serial.println("Blynk: Configuring New Blynk cloud...");
  Blynk.config(BLYNK_AUTH_TOKEN, "blynk.cloud", 80);
  // Removed Blynk.connect() to prevent blocking during initialization.
  // Blynk.run() will attempt to connect in the background.
  Serial.println("Blynk: Setup complete (will connect in background)");
}

void blynkPublishTelemetry(int gas, float distance, bool motion, int distanceLevel){
  if (!Blynk.connected()) return;
  
  Blynk.virtualWrite(V0, gas);
  if (distance < 0) Blynk.virtualWrite(V1, "--");
  else Blynk.virtualWrite(V1, (int)distance);
  Blynk.virtualWrite(V2, motion ? 1 : 0);
  Blynk.virtualWrite(V4, distanceLevel);
}

void blynkNotify(const char* msg){
  if (!Blynk.connected()) return;
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
