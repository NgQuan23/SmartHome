#pragma once
#include <Arduino.h>

void blynkInit();
void blynkPublishTelemetry(int gas, float distance, bool motion, int distanceLevel);
void blynkNotify(const char* msg);
void blynkRun();
