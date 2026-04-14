#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "config.h"
#include "wifi_mqtt.h"
#include "ota.h"
#include <ArduinoJson.h>
#include <ArduinoOTA.h>
#include "firebase_client.h"
#include "telegram.h"
#include "storage.h"
#include "esp_sleep.h"

LiquidCrystal_I2C lcd(0x27, 16, 2);


const int GAS_THRESHOLD = 1500; 
const int MQ2_SAMPLES = 8;      
const unsigned long PIR_COOLDOWN_MS = 3000;
const unsigned long SENSOR_POLL_MS = 1000;   
const unsigned long STORAGE_FLUSH_MS = 30000;


void readSensors();
float getDistance();
int readMQ2Avg();


unsigned long lastPirTime = 0;
unsigned long lastPoll = 0;
unsigned long lastAlertTime = 0;
const unsigned long ALERT_COOLDOWN_MS = 60000; 

void setup() {
  Serial.begin(115200);
  delay(5000); // 5 seconds wait for USB CDC attachment
  Serial.println("\n\n=== SmartHome Starting ===");
  Serial.flush();

#ifdef ESP32
  // analogReadResolution(12);
#endif

  Serial.println("Setting up pins...");
  pinMode(PIN_PIR, INPUT);
  pinMode(PIN_TRIG, OUTPUT);
  pinMode(PIN_ECHO, INPUT);
  pinMode(PIN_BUZZER, OUTPUT);
  pinMode(PIN_RELAY, OUTPUT);
  pinMode(PIN_VALVE_RELAY, OUTPUT);


  digitalWrite(PIN_RELAY, LOW);
  digitalWrite(PIN_BUZZER, LOW);

  Serial.println("Initializing LCD...");
  Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);
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
  storageInit();
  storageFlush();  // Flush stored events after Firebase is ready
  otaInit();
}

void loop() {
  unsigned long now = millis();
  if (now - lastPoll >= SENSOR_POLL_MS) {
    lastPoll = now;
    readSensors();
  }

  mqttLoop();
  if (WiFi.status() == WL_CONNECTED) ArduinoOTA.handle();

  static unsigned long lastStorageFlush = 0;
  if (now - lastStorageFlush >= STORAGE_FLUSH_MS) {
    lastStorageFlush = now;
    storageFlush();
  }


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
  bool awayMode = firebaseGetAwayMode();
  bool motion = awayMode ? digitalRead(PIN_PIR) : false;

  
  int distCritical = firebaseGetDistanceCritical();
  if (distCritical <= 0) distCritical = DIST_LEVEL_2;

  int gasWarning = firebaseGetGasWarning();
  if (gasWarning <= 0) gasWarning = GAS_LEVEL_2;

  int distanceLevel = 0;
  if (distance >= 0) {
    if (distance <= DIST_LEVEL_3) distanceLevel = 3;
    else if (distance <= distCritical) distanceLevel = 2;
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
  (void)n;
  String telemetryPayload(buf);
  bool mqttOk = mqttPublish(TELEMETRY_TOPIC, telemetryPayload.c_str());
  firebasePushTelemetryPayload(telemetryPayload, true);
  if (!mqttOk) {
    storageAppend(telemetryPayload);
  }


  unsigned long nowAlert = millis();
  String msgQueue[3];
  int msgCount = 0;

  if (gasValue >= GAS_LEVEL_3) {
    msgQueue[msgCount++] = "FIRE! LOCK VAL";
    digitalWrite(PIN_VALVE_RELAY, HIGH); 
    digitalWrite(PIN_FAN_RELAY, LOW); 
    digitalWrite(PIN_BUZZER, HIGH);
    if ((nowAlert - lastAlertTime) >= ALERT_COOLDOWN_MS) {
      lastAlertTime = nowAlert;
      String msg = String("EMERGENCY: FIRE detected! Gas=") + gasValue;
      telegramSend(msg);
    }
  }
  else if (gasValue >= gasWarning) {
    msgQueue[msgCount++] = "LEAK: FAN ON";
    digitalWrite(PIN_FAN_RELAY, HIGH);
    digitalWrite(PIN_BUZZER, LOW);
    if ((nowAlert - lastAlertTime) >= ALERT_COOLDOWN_MS) {
      lastAlertTime = nowAlert;
      String msg = String("Warning: Gas leak detected. Value=") + gasValue;
      telegramSend(msg);
    }
  }
  else if (gasValue >= GAS_LEVEL_1) {
    msgQueue[msgCount++] = "Leak: Low";
    digitalWrite(PIN_FAN_RELAY, LOW);
    digitalWrite(PIN_BUZZER, LOW);
  }
  else {
    // Không có lỗi Gas
    digitalWrite(PIN_FAN_RELAY, LOW);
    digitalWrite(PIN_VALVE_RELAY, LOW);
    digitalWrite(PIN_BUZZER, LOW);
  }



  if (distance > 0 && distance <= DIST_LEVEL_3) {
    msgQueue[msgCount++] = "FLOOD: CRITICAL!";
    for (int i = 0; i < 5; ++i) {
      digitalWrite(PIN_BUZZER, HIGH);
      delay(150);
      digitalWrite(PIN_BUZZER, LOW);
      delay(100);
    }
  
    digitalWrite(PIN_RELAY, HIGH);
    if ((nowAlert - lastAlertTime) >= ALERT_COOLDOWN_MS) {
      lastAlertTime = nowAlert;
      String msg = String("EMERGENCY: Flood! Water level critical (") + (int)distance + "cm). Cutting power.";
      telegramSend(msg);
    }
  }
  else if (distance > 0 && distance <= distCritical) {
    msgQueue[msgCount++] = "WATER LEVEL HIGH";
    for (int i = 0; i < 3; ++i) {
      digitalWrite(PIN_BUZZER, HIGH);
      delay(120);
      digitalWrite(PIN_BUZZER, LOW);
      delay(120);
    }
    if ((nowAlert - lastAlertTime) >= ALERT_COOLDOWN_MS) {
      lastAlertTime = nowAlert;
      String msg = String("Warning: Water level is high (") + (int)distance + "cm)";
      telegramSend(msg);
    }
  }
  // Removing WATER RISING msg to only alert when water level is critically high (<= distCritical)

  lcd.setCursor(0, 0);
  lcd.print("G:"); lcd.print(gasValue);
  lcd.print(" ");
  lcd.print("D:");
  if (distance < 0) lcd.print("--cm    ");
  else {
    String distStr = String((int)distance) + "/" + String(distCritical) + "cm";
    lcd.print(distStr);

    int pad = 8 - distStr.length();
    if (pad > 0) {
      for (int i = 0; i < pad; ++i) lcd.print(' ');
    }
  }

  
  if (motion && awayMode) {
    msgQueue[msgCount++] = "INTRUDER ALERT!";
    digitalWrite(PIN_BUZZER, HIGH);
    delay(150);
    digitalWrite(PIN_BUZZER, LOW);
    digitalWrite(PIN_RELAY, LOW);
  }
  else if (gasValue > GAS_THRESHOLD) {
    digitalWrite(PIN_BUZZER, HIGH);
    digitalWrite(PIN_RELAY, HIGH);
  }
  else {
    digitalWrite(PIN_BUZZER, LOW);
    digitalWrite(PIN_RELAY, LOW);
  }

  // Khối xử lý hiển thị xoay vòng màn hình LCD
  lcd.setCursor(0, 1);
  if (msgCount == 0) {
    lcd.print("Status: Safe    ");
  } else {
    static int currentMsgIdx = 0;
    if (currentMsgIdx >= msgCount) currentMsgIdx = 0;
    
    String dispMsg = msgQueue[currentMsgIdx];
    lcd.print(dispMsg);
    
    // Độn đuôi khoảng trắng (Pad) để xóa các kí tự thừa 
    int len = dispMsg.length();
    for (int i = 0; i < 16 - len; i++) {
       lcd.print(" ");
    }
    
    currentMsgIdx++;
  }
}


int readMQ2Avg(){
  long sum = 0;
  for (int i = 0; i < MQ2_SAMPLES; ++i) {
    sum += (4095 - analogRead(PIN_MQ2)); // Đảo ngược logic giá trị cảm biến
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
