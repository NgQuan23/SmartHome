#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "config.h"
#include "wifi_mqtt.h"
#include "ota.h"
#include <ArduinoJson.h>
#include <ArduinoOTA.h>
#include "firebase_client.h"
#include "blynk_client.h"
#include "telegram.h"
#include "storage.h"
#include "esp_sleep.h"

#define PIN_MQ2       34 // analog input (ESP32 ADC1)
#define PIN_PIR       27 // digital input
#define PIN_TRIG      5  // HC-SR04 trigger
#define PIN_ECHO      18 // HC-SR04 echo (through voltage divider in hardware)
#define PIN_BUZZER    13 // active HIGH buzzer
#define PIN_RELAY     14 // relay module control (active HIGH)

LiquidCrystal_I2C lcd(0x27, 16, 2);

// Configuration / thresholds
const int GAS_THRESHOLD = 1500; // adjustable: raw ADC value (0-4095 on ESP32)
const int MQ2_SAMPLES = 8;      // number of samples to average
const unsigned long PIR_COOLDOWN_MS = 3000; // ignore PIR retriggers for this period
const unsigned long SENSOR_POLL_MS = 500;   // main loop interval

// Function prototypes
void readSensors();
float getDistance();
int readMQ2Avg();

// runtime state
unsigned long lastPirTime = 0;
unsigned long lastPoll = 0;
unsigned long lastAlertTime = 0;
const unsigned long ALERT_COOLDOWN_MS = 60000; // 60s between notifications

void setup() {
  Serial.begin(115200);

#ifdef ESP32
  // ensure MQ2 ADC pin uses full range ~0-3.3V (set attenuation)
  // ADC_11db gives full range ~3.3V on most ESP32 cores
  analogSetPinAttenuation(PIN_MQ2, ADC_11db);
#endif

  pinMode(PIN_MQ2, INPUT);
  pinMode(PIN_PIR, INPUT);
  pinMode(PIN_TRIG, OUTPUT);
  pinMode(PIN_ECHO, INPUT);
  pinMode(PIN_BUZZER, OUTPUT);
  pinMode(PIN_RELAY, OUTPUT);
  pinMode(PIN_VALVE_RELAY, OUTPUT);

  // default: relay off, buzzer off
  digitalWrite(PIN_RELAY, LOW);
  digitalWrite(PIN_BUZZER, LOW);

  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Security System");
  lcd.setCursor(0, 1);
  lcd.print("Ready...");
  delay(1200);
  lcd.clear();

  // Networking and OTA
  wifiInit();
  mqttInit();
  // initialize Firebase and Blynk (if enabled)
  firebaseInit();
  blynkInit();
  storageInit();
  otaInit();
}

void loop() {
  unsigned long now = millis();
  if (now - lastPoll >= SENSOR_POLL_MS) {
    lastPoll = now;
    readSensors();
  }
  // keep MQTT/OTA responsive
  mqttLoop();
  // Blynk needs running in loop when enabled (use wrapper)
  #if ENABLE_BLYNK
  blynkRun();
  #endif
  if (WiFi.status() == WL_CONNECTED) ArduinoOTA.handle();

  // deep sleep check (idle)
  static unsigned long lastActive = 0;
  if (ENABLE_DEEP_SLEEP) {
    if ((now - lastActive) > IDLE_TIMEOUT_MS) {
      Serial.println("Entering deep sleep due to inactivity");
      // configure wakeup: timer (30s) and ext0 by PIR (if supported)
      esp_sleep_enable_timer_wakeup(30ULL * 1000000ULL);
      // ext0 wake on PIR HIGH (if RTC IO); pin mapping depends on board
      // esp_sleep_enable_ext0_wakeup((gpio_num_t)PIN_PIR, 1);
      esp_deep_sleep_start();
    }
  }
}

// read sensors, update LCD, serial, and actuators
void readSensors(){
  int gasValue = readMQ2Avg();
  float distance = getDistance();
  bool motion = digitalRead(PIN_PIR);

  // compute distance alert level (0 = none, 1..3)
  int distanceLevel = 0;
  if (distance >= 0) {
    if (distance <= DIST_LEVEL_3) distanceLevel = 3;
    else if (distance <= DIST_LEVEL_2) distanceLevel = 2;
    else if (distance <= DIST_LEVEL_1) distanceLevel = 1;
  }

  // PIR cooldown handling to reduce repeated alerts
  if (motion) {
    unsigned long now = millis();
    if (now - lastPirTime < PIR_COOLDOWN_MS) {
      motion = false; // ignore repeated triggers within cooldown
    } else {
      lastPirTime = now;
    }
  }

  // update activity timestamp for deep sleep logic
  static unsigned long lastActiveLocal = 0;
  if (motion || gasValue > GAS_LEVEL_1) lastActiveLocal = millis();

  // Log to serial
  Serial.printf("Gas: %d | Distance: %s | Motion: %s\n", gasValue,
                (distance < 0) ? "Out" : String(distance, 1).c_str(),
                motion ? "DETECTED" : "NONE");

  // publish telemetry (lightweight JSON)
  StaticJsonDocument<192> doc;
  doc["gas"] = gasValue;
  doc["distance"] = (distance < 0) ? -1 : distance;
  doc["distance_level"] = distanceLevel;
  doc["motion"] = motion ? 1 : 0;
  char buf[192];
  size_t n = serializeJson(doc, buf);
  bool ok = mqttPublish(TELEMETRY_TOPIC, buf);
  if (!ok) storageAppend(String(buf));

  // publish to Firebase and Blynk
  firebasePushTelemetry(gasValue, distance, motion);
  blynkPublishTelemetry(gasValue, distance, motion, distanceLevel);

  // MQ-2 levels handling
  unsigned long nowAlert = millis();
  if (gasValue >= GAS_LEVEL_3) {
    // Level 3: fire / emergency
    lcd.setCursor(0,1); lcd.print("FIRE! LOCK VALVE");
    digitalWrite(PIN_VALVE_RELAY, HIGH); // lock/close valve
    digitalWrite(PIN_FAN_RELAY, LOW); // optionally stop fan
    digitalWrite(PIN_BUZZER, HIGH);
    if ((nowAlert - lastAlertTime) >= ALERT_COOLDOWN_MS) {
      lastAlertTime = nowAlert;
      String msg = String("EMERGENCY: FIRE detected! Gas=") + gasValue;
      telegramSend(msg);
      blynkNotify(msg.c_str());
    }
  }
  else if (gasValue >= GAS_LEVEL_2) {
    // Level 2: higher leak -> turn on fan and light alert
    lcd.setCursor(0,1); lcd.print("LEAK: FAN ON     ");
    digitalWrite(PIN_FAN_RELAY, HIGH);
    digitalWrite(PIN_BUZZER, LOW);
    if ((nowAlert - lastAlertTime) >= ALERT_COOLDOWN_MS) {
      lastAlertTime = nowAlert;
      String msg = String("Warning: Gas leak detected. Value=") + gasValue;
      telegramSend(msg);
      blynkNotify(msg.c_str());
    }
  }
  else if (gasValue >= GAS_LEVEL_1) {
    // Level 1: light leak
    lcd.setCursor(0,1); lcd.print("Leak: Low       ");
    digitalWrite(PIN_FAN_RELAY, LOW);
    digitalWrite(PIN_BUZZER, LOW);
  }
  else {
    // Safe
    lcd.setCursor(0,1); lcd.print("Status: Safe   ");
    digitalWrite(PIN_FAN_RELAY, LOW);
    digitalWrite(PIN_VALVE_RELAY, LOW);
    digitalWrite(PIN_BUZZER, LOW);
  }

  // Distance levels handling (proximity alerts)
  if (distanceLevel == 3) {
    // Level 3: very close -> emergency
    lcd.setCursor(0,1); lcd.print("OBJ VERY CLOSE   ");
    digitalWrite(PIN_BUZZER, HIGH);
    // Optionally activate relay as a visible alarm
    digitalWrite(PIN_RELAY, HIGH);
    if ((nowAlert - lastAlertTime) >= ALERT_COOLDOWN_MS) {
      lastAlertTime = nowAlert;
      String msg = String("EMERGENCY: object very close (") + (int)distance + "cm)";
      telegramSend(msg);
      blynkNotify(msg.c_str());
    }
  }
  else if (distanceLevel == 2) {
    // Level 2: near -> warning
    lcd.setCursor(0,1); lcd.print("OBJ NEAR         ");
    // short beep
    digitalWrite(PIN_BUZZER, HIGH);
    delay(120);
    digitalWrite(PIN_BUZZER, LOW);
    if ((nowAlert - lastAlertTime) >= ALERT_COOLDOWN_MS) {
      lastAlertTime = nowAlert;
      String msg = String("Warning: object near (") + (int)distance + "cm)";
      telegramSend(msg);
      blynkNotify(msg.c_str());
    }
  }
  else if (distanceLevel == 1) {
    // Level 1: caution
    lcd.setCursor(0,1); lcd.print("OBJ DETECTED     ");
    // no loud alarm, just update display
  }

  // Update LCD (line 1: gas and distance)
  lcd.setCursor(0, 0);
  lcd.print("G:"); lcd.print(gasValue);
  lcd.print(" ");
  lcd.print("D:");
  if (distance < 0) lcd.print("--cm   ");
  else {
    // ensure consistent width
    lcd.print((int)distance);
    lcd.print("cm");
    // pad if needed
    int pad = 6 - String((int)distance).length();
    for (int i = 0; i < pad; ++i) lcd.print(' ');
  }

  // Update LCD (line 2: status) and actuators
  lcd.setCursor(0, 1);
  if (gasValue > GAS_THRESHOLD) {
    lcd.print("ALERT: GAS!    ");
    digitalWrite(PIN_BUZZER, HIGH);
    digitalWrite(PIN_RELAY, HIGH); // example: activate relay to cut power/vent
  }
  else if (motion) {
    lcd.print("PIR: ALERT!    ");
    // brief beep to indicate motion (blocking small delay for clarity)
    digitalWrite(PIN_BUZZER, HIGH);
    delay(150);
    digitalWrite(PIN_BUZZER, LOW);
    digitalWrite(PIN_RELAY, LOW);
  }
  else {
    lcd.print("Status: Safe   ");
    digitalWrite(PIN_BUZZER, LOW);
    digitalWrite(PIN_RELAY, LOW);
  }
}

// read MQ2 multiple times and return averaged ADC raw value
int readMQ2Avg(){
  long sum = 0;
  for (int i = 0; i < MQ2_SAMPLES; ++i) {
    sum += analogRead(PIN_MQ2);
    delay(5);
  }
  return (int)(sum / MQ2_SAMPLES);
}

// measure distance in cm; returns -1.0f on timeout/no-echo
float getDistance(){
  digitalWrite(PIN_TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(PIN_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(PIN_TRIG, LOW);

  unsigned long duration = pulseIn(PIN_ECHO, HIGH, 30000UL); // timeout 30ms
  if (duration == 0UL) {
    return -1.0f;
  }

  return (duration * 0.034f) / 2.0f;
}