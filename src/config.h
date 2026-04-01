
#ifndef CONFIG_H
#define CONFIG_H


#define WIFI_SSID "Quan"
#define WIFI_PASS "88888888"


#define MQTT_SERVER "broker.hivemq.com"
#define MQTT_PORT 1883
#define MQTT_USER ""
#define MQTT_PASSWORD ""


#define TELEMETRY_TOPIC "smarthome/device1/telemetry"
#define COMMAND_TOPIC   "smarthome/device1/command"


#define OTA_HOSTNAME "SmartHome_Device"


#define FIREBASE_DATABASE_URL "https://smart-home-1c235.firebaseio.com"
#define FIREBASE_PROJECT_ID "smart-home-1c235"
#define FIREBASE_API_KEY "AIzaSyAPHaUEbzXHmLfCRPRqJ27c4NPZIVfFyk8"


#define FIREBASE_HOST "smart-home-1c235.firebaseio.com"


#define FIREBASE_USER_EMAIL "quanylksnb@gmail.com"
#define FIREBASE_USER_PASSWORD "0912848144"


#define FIREBASE_SERVICE_ACCOUNT_PATH "" 


#define FIREBASE_DATABASE_SECRET ""


#define BLYNK_TEMPLATE_ID "TMPL66UiZuFxu"
#define BLYNK_TEMPLATE_NAME "smart home"
#define BLYNK_AUTH_TOKEN "sc269hJiU_myc4vr0j_ckW3cGh0_fqox"


#define TG_BOT_TOKEN "8527377630:AAFaNC761ehG82QhE34nb4voTxO7GM--zSk"
#define TG_CHAT_ID "6578744940"


#define ENABLE_FIREBASE 1
#define ENABLE_BLYNK 1
#define ENABLE_TELEGRAM 1


#define GAS_LEVEL_1 800   
#define GAS_LEVEL_2 1400  
#define GAS_LEVEL_3 3000  


#define DIST_LEVEL_1 150  
#define DIST_LEVEL_2 5    
#define DIST_LEVEL_3 2    


#define PIN_FAN_RELAY PIN_RELAY
#define PIN_VALVE_RELAY 17 


#define ENABLE_DEEP_SLEEP 1
#define IDLE_TIMEOUT_MS (5 * 60 * 1000UL) 

#endif 
