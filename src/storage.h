#pragma once
#include <Arduino.h>

void storageInit();
void storageAppend(const String &jsonLine);
void storageFlush();
