#pragma once
#include <Arduino.h>

void firebaseInit();
bool firebasePushTelemetry(int gas, float distance, bool motion, bool queueOnFailure = true);
bool firebasePushTelemetryPayload(const String &payload, bool queueOnFailure = true);
bool firebaseIsReady();
bool firebaseGetAwayMode();
int firebaseGetDistanceCritical();
int firebaseGetGasWarning();
