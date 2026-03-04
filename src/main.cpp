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

// ESP32-S3 compatible pins
#define PIN_MQ2       4  // ADC1 channel on ESP32-S3
#define PIN_PIR       5  
#define PIN_TRIG      6  
#define PIN_ECHO      7 
#define PIN_BUZZER    15 
#define PIN_RELAY     16 

LiquidCrystal_I2C lcd(0x27, 16, 2);


const int GAS_THRESHOLD = 1500; 
const int MQ2_SAMPLES = 8;      
const unsigned long PIR_COOLDOWN_MS = 3000; 
const unsigned long SENSOR_POLL_MS = 500;   


void readSensors();
float getDistance();
int readMQ2Avg();


unsigned long lastPirTime = 0;
unsigned long lastPoll = 0;
unsigned long lastAlertTime = 0;
const unsigned long ALERT_COOLDOWN_MS = 60000; 

void setup() {
  Serial.begin(115200);
  delay(1000);  // Wait for serial monitor
  Serial.println("\n\n=== SmartHome Starting ===");
  Serial.flush();

#if defined(ESP32) && !defined(CONFIG_IDF_TARGET_ESP32S3)
  // Only for original ESP32, not ESP32-S3
  analogSetPinAttenuation(PIN_MQ2, ADC_11db);
#endif

  Serial.println("Setting up pins...");
  pinMode(PIN_MQ2, INPUT);
  pinMode(PIN_PIR, INPUT);
  pinMode(PIN_TRIG, OUTPUT);
  pinMode(PIN_ECHO, INPUT);
  pinMode(PIN_BUZZER, OUTPUT);
  pinMode(PIN_RELAY, OUTPUT);
  pinMode(PIN_VALVE_RELAY, OUTPUT);


  digitalWrite(PIN_RELAY, LOW);
  digitalWrite(PIN_BUZZER, LOW);

  Serial.println("Initializing LCD...");
  Wire.begin(8, 9);  // ESP32-S3 default I2C pins: SDA=8, SCL=9
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Security System");
  lcd.setCursor(0, 1);
  lcd.print("Ready...");
  delay(1200);
  lcd.clear();
  Serial.println("LCD initialized");


  Serial.println("Connecting to WiFi...");
  wifiInit();
  mqttInit();

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

  mqttLoop();

  #if ENABLE_BLYNK
  blynkRun();
  #endif
  if (WiFi.status() == WL_CONNECTED) ArduinoOTA.handle();


  static unsigned long lastActive = 0;
  if (ENABLE_DEEP_SLEEP) {
    if ((now - lastActive) > IDLE_TIMEOUT_MS) {
      Serial.println("Entering deep sleep due to inactivity");
    
      esp_sleep_enable_timer_wakeup(30ULL * 1000000ULL);

      esp_deep_sleep_start();
    }
  }
}


void readSensors(){
  int gasValue = readMQ2Avg();
  float distance = getDistance();
  bool motion = digitalRead(PIN_PIR);

  
  int distanceLevel = 0;
  if (distance >= 0) {
    if (distance <= DIST_LEVEL_3) distanceLevel = 3;
    else if (distance <= DIST_LEVEL_2) distanceLevel = 2;
    else if (distance <= DIST_LEVEL_1) distanceLevel = 1;
  }


  if (motion) {
    unsigned long now = millis();
    if (now - lastPirTime < PIR_COOLDOWN_MS) {
      motion = false; 
    } else {
      lastPirTime = now;
    }
  }


  static unsigned long lastActiveLocal = 0;
  if (motion || gasValue > GAS_LEVEL_1) lastActiveLocal = millis();


  Serial.printf("Gas: %d | Distance: %s | Motion: %s\n", gasValue,
                (distance < 0) ? "Out" : String(distance, 1).c_str(),
                motion ? "DETECTED" : "NONE");


  StaticJsonDocument<192> doc;
  doc["gas"] = gasValue;
  doc["distance"] = (distance < 0) ? -1 : distance;
  doc["distance_level"] = distanceLevel;
  doc["motion"] = motion ? 1 : 0;
  char buf[192];
  size_t n = serializeJson(doc, buf);
  bool ok = mqttPublish(TELEMETRY_TOPIC, buf);
  if (!ok) storageAppend(String(buf));


  firebasePushTelemetry(gasValue, distance, motion);
  blynkPublishTelemetry(gasValue, distance, motion, distanceLevel);


  unsigned long nowAlert = millis();
  if (gasValue >= GAS_LEVEL_3) {

    lcd.setCursor(0,1); lcd.print("FIRE! LOCK VAL");
    digitalWrite(PIN_VALVE_RELAY, HIGH); 
    digitalWrite(PIN_FAN_RELAY, LOW); 
    digitalWrite(PIN_BUZZER, HIGH);
    if ((nowAlert - lastAlertTime) >= ALERT_COOLDOWN_MS) {
      lastAlertTime = nowAlert;
      String msg = String("EMERGENCY: FIRE detected! Gas=") + gasValue;
      telegramSend(msg);
      blynkNotify(msg.c_str());
    }
  }
  else if (gasValue >= GAS_LEVEL_2) {
  
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
  
    lcd.setCursor(0,1); lcd.print("Leak: Low       ");
    digitalWrite(PIN_FAN_RELAY, LOW);
    digitalWrite(PIN_BUZZER, LOW);
  }
  else {
  
    lcd.setCursor(0,1); lcd.print("Status: Safe   ");
    digitalWrite(PIN_FAN_RELAY, LOW);
    digitalWrite(PIN_VALVE_RELAY, LOW);
    digitalWrite(PIN_BUZZER, LOW);
  }



  if (distance > 0 && distance <= DIST_LEVEL_3) {
    lcd.setCursor(0,1); lcd.print("CRITICAL: OBSTACLE");
    for (int i = 0; i < 5; ++i) {
      digitalWrite(PIN_BUZZER, HIGH);
      delay(150);
      digitalWrite(PIN_BUZZER, LOW);
      delay(100);
    }
  
    digitalWrite(PIN_RELAY, HIGH);
    if ((nowAlert - lastAlertTime) >= ALERT_COOLDOWN_MS) {
      lastAlertTime = nowAlert;
      String msg = String("CRITICAL: object too close (") + (int)distance + "cm). Cutting power.";
      telegramSend(msg);
      blynkNotify(msg.c_str()); 
    }
  }
  else if (distance > 0 && distance <= DIST_LEVEL_2) {
    lcd.setCursor(0,1); lcd.print("WARNING: NEAR    ");
    for (int i = 0; i < 3; ++i) {
      digitalWrite(PIN_BUZZER, HIGH);
      delay(120);
      digitalWrite(PIN_BUZZER, LOW);
      delay(120);
    }
    if ((nowAlert - lastAlertTime) >= ALERT_COOLDOWN_MS) {
      lastAlertTime = nowAlert;
      String msg = String("Warning: object near (") + (int)distance + "cm)";
      telegramSend(msg);
      blynkNotify(msg.c_str());
    }
  }
  else if (distanceLevel == 1) {
    lcd.setCursor(0,1); lcd.print("HUMAN DETECTED ");
  }


  lcd.setCursor(0, 0);
  lcd.print("G:"); lcd.print(gasValue);
  lcd.print(" ");
  lcd.print("D:");
  if (distance < 0) lcd.print("--cm   ");
  else {

    lcd.print((int)distance);
    lcd.print("cm");

    int pad = 6 - String((int)distance).length();
    for (int i = 0; i < pad; ++i) lcd.print(' ');
  }

  
  lcd.setCursor(0, 1);
  if (gasValue > GAS_THRESHOLD) {
    lcd.print("ALERT: GAS!    ");
    digitalWrite(PIN_BUZZER, HIGH);
    digitalWrite(PIN_RELAY, HIGH);
  }
  else if (motion) {
    lcd.print("PIR: ALERT!    ");

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


int readMQ2Avg(){
  long sum = 0;
  for (int i = 0; i < MQ2_SAMPLES; ++i) {
    sum += analogRead(PIN_MQ2);
    delay(5);
  }
  return (int)(sum / MQ2_SAMPLES);
}


float getDistance(){
  digitalWrite(PIN_TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(PIN_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(PIN_TRIG, LOW);

  unsigned long duration = pulseIn(PIN_ECHO, HIGH, 30000UL);
  if (duration == 0UL) {
    return -1.0f;
  }

  return (duration * 0.034f) / 2.0f;
}