// config.h - project configuration placeholders
#ifndef CONFIG_H
#define CONFIG_H

// WiFi credentials (replace with your network)
#define WIFI_SSID "YOUR_SSID"
#define WIFI_PASS "YOUR_PASS"

// MQTT (or broker) settings
#define MQTT_SERVER "mqtt.example.com"
#define MQTT_PORT 1883
#define MQTT_USER ""
#define MQTT_PASSWORD ""

// Telemetry topic (device-specific)
#define TELEMETRY_TOPIC "smarthome/device1/telemetry"
#define COMMAND_TOPIC   "smarthome/device1/command"

// OTA
#define OTA_HOSTNAME "SmartHome_Device"

// Firebase (Realtime Database) - replace with your project values
// Example: "https://your-project.firebaseio.com"
#define FIREBASE_HOST "https://your-project.firebaseio.com"
#define FIREBASE_API_KEY "YOUR_FIREBASE_API_KEY"

// Blynk
#define BLYNK_AUTH "YOUR_BLYNK_AUTH_TOKEN"
// For Blynk v1.3 template (optional): define these if using Blynk new templates
#define BLYNK_TEMPLATE_ID ""
#define BLYNK_TEMPLATE_NAME ""

// Telegram Bot
#define TG_BOT_TOKEN "YOUR_TELEGRAM_BOT_TOKEN"
#define TG_CHAT_ID "YOUR_TELEGRAM_CHAT_ID"

// Feature toggles
#define ENABLE_FIREBASE 1
#define ENABLE_BLYNK 1
#define ENABLE_TELEGRAM 1

// MQ-2 thresholds (ADC raw 0-4095)
#define GAS_LEVEL_1 800   // light leak
#define GAS_LEVEL_2 1400  // stronger leak -> activate fan
#define GAS_LEVEL_3 3000  // fire / very high -> emergency

// HC-SR04 distance alert thresholds (cm)
// Feel free to tune these for your environment
#define DIST_LEVEL_1 150  // object within 150cm -> caution
#define DIST_LEVEL_2 80   // object within 80cm -> near
#define DIST_LEVEL_3 30   // object within 30cm -> very close / emergency

// Relays pins
#define PIN_FAN_RELAY PIN_RELAY
#define PIN_VALVE_RELAY 15 // change to your actual valve control pin

// Deep sleep / power saving
#define ENABLE_DEEP_SLEEP 1
#define IDLE_TIMEOUT_MS (5 * 60 * 1000UL) // 5 minutes idle before deep sleep

#endif // CONFIG_H
