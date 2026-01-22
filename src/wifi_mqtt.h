#pragma once
#include <Arduino.h>

void wifiInit();
bool mqttPublish(const char* topic, const char* payload);
void mqttLoop();
void mqttInit();
