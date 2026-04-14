
#ifndef CONFIG_H
#define CONFIG_H


#define WIFI_SSID "parkytown"
#define WIFI_PASS "thisismyserver"


#define MQTT_SERVER "broker.hivemq.com"
#define MQTT_PORT 1883
#define MQTT_USER ""
#define MQTT_PASSWORD ""


#define TELEMETRY_TOPIC "smarthome/device1/telemetry"
#define COMMAND_TOPIC   "smarthome/device1/command"


#define OTA_HOSTNAME "SmartHome_Device"


#define FIREBASE_DATABASE_URL "https://smart-home-1c235-default-rtdb.asia-southeast1.firebasedatabase.app"
#define FIREBASE_PROJECT_ID "smart-home-1c235"
#define FIREBASE_API_KEY "AIzaSyAPHaUEbzXHmLfCRPRqJ27c4NPZIVfFyk8"


#define FIREBASE_HOST "smart-home-1c235-default-rtdb.asia-southeast1.firebasedatabase.app"


#define FIREBASE_USER_EMAIL "quanylksnb@gmail.com"
#define FIREBASE_USER_PASSWORD "0912848144"


#define FIREBASE_SERVICE_ACCOUNT_PATH "" 


#define FIREBASE_DATABASE_SECRET ""


#define TG_BOT_TOKEN "8527377630:AAFaNC761ehG82QhE34nb4voTxO7GM--zSk"
#define TG_CHAT_ID "6578744940"


#define ENABLE_FIREBASE 1
#define ENABLE_TELEGRAM 1

// ESP32-S3 Super Mini wiring map for the SmartHome board.
// Avoid GPIO0/GPIO3/GPIO19/GPIO20/GPIO45/GPIO46 and flash-connected GPIO9-14.
#define PIN_MQ2 1
#define PIN_PIR 4
#define PIN_TRIG 2
#define PIN_ECHO 3
#define PIN_I2C_SDA 8
#define PIN_BUZZER 5
#define PIN_RELAY 16
#define PIN_FAN_RELAY PIN_RELAY
#define PIN_VALVE_RELAY 17
#define PIN_I2C_SCL 9


#define GAS_LEVEL_1 800   
#define GAS_LEVEL_2 1400  
#define GAS_LEVEL_3 3000  


#define DIST_LEVEL_1 150  
#define DIST_LEVEL_2 5    
#define DIST_LEVEL_3 2    


#define ENABLE_DEEP_SLEEP 0
#define IDLE_TIMEOUT_MS (5 * 60 * 1000UL) 

#endif 
