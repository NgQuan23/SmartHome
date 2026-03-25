#pragma once
#include <Arduino.h>

void storageInit();
bool storageAppend(const String &jsonLine);
void storageFlush();
bool storageAvailable();
