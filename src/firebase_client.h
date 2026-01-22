#pragma once
#include <Arduino.h>

void firebaseInit();
void firebasePushTelemetry(int gas, float distance, bool motion);
