# Smart Home Security & Monitoring System

This project is an ESP32-S3 based Smart Home system focusing on security, environmental monitoring, and remote alerting.

## 🌟 Key Features

### 1. Gas Leak & Fire Detection (MQ-2 Sensor)
- **Level 1 (Warning):** Detects slight gas presence. Local LCD warning.
- **Level 2 (Leak):** Triggers exhaust fan relay to clear air, sends mobile notifications.
- **Level 3 (Fire/Emergency):** Triggers loud buzzer, cuts gas valve (relay), shows critical alert on LCD, and sends immediate push notifications.

### 2. Intrusion & Proximity Alert (PIR & HC-SR04)
- **Human Detection (PIR):** Detects motion; wakes screen and sends alerts.
- **Proximity Alert (Ultrasonic):** 
  - *Warning:* Object near (Warning beep & notification).
  - *Critical:* Object too close (Constant alarm & cuts main power via relay).

### 3. Multi-Channel Notifications
- **Local Alerts:** 16x2 I2C LCD Display and active Buzzer.
- **Telegram Bot:** Real-time chat messages for alerts.
- **Blynk IoT:** Mobile app dashboard for real-time telemetry (Virtual Pins V0-V4) and popup notifications.

### 4. Cloud & Data Integrations
- **Firebase RTDB:** Logs sensor data directly to a Firebase project.
- **MQTT:** Publishes JSON telemetry data to an MQTT broker (`smarthome/device1/telemetry`) and subscribes to commands.
- **Offline Storage:** Queues messages locally if WiFi drops, and resends them when back online.

### 5. Advanced System Features
- **Deep Sleep:** Enters low power mode after 5 minutes of inactivity to save energy.
- **OTA Updates:** Supports remote over-the-air firmware updates via ArduinoOTA.

---

## 🛠 Project Configuration

All major configurations (WiFi, MQTT, Firebase, Blynk, Telegram) are located in `src/config.h`. 

### Configuring Blynk IoT (New Version)
This project uses the modern **Blynk IoT** platform (non-blocking initialization). To set it up:

1. **Create a Template:** Go to the [Blynk Web Console](https://blynk.cloud/) and create a new Template named `smart home` (Hardware: ESP32, Connection: WiFi).
2. **Define Datastreams:** Create the following Virtual Pins in your template:
   - `V0` (Integer): Gas Level
   - `V1` (Integer/String): Distance (cm)
   - `V2` (Integer): Motion detected (1/0)
   - `V3` (String): Alert Messages
   - `V4` (Integer): Distance Alert Level (1-3)
3. **Copy Credentials:** Once you add a new Device from this template, copy the credentials block provided by Blynk and paste it into `src/config.h`:
   ```cpp
   #define BLYNK_TEMPLATE_ID "TMPLxxxxxx"
   #define BLYNK_TEMPLATE_NAME "smart home"
   #define BLYNK_AUTH_TOKEN "Your_Auth_Token_Here"
   ```
4. **Compile & Upload:** The system will automatically use these credentials to securely connect to the Blynk server without blocking other tasks like MQTT or Firebase.

---

### Hardware Usage Note (ESP32-S3)
If you are facing issues uploading code (e.g. `pio run -t upload` fails), it is usually because the ESP32-S3 native USB COM port has not been recognized. 
**To Fix Uploading:**
1. Connect the ESP32-S3 to your PC via a data USB cable.
2. Hold down the **BOOT** button (usually marked 0 or BOOT) on the board.
3. While holding **BOOT**, click the **EN** (or RST) button once.
4. Release the **BOOT** button.
5. Your computer should now recognize the COM port, and `pio run -t upload` will succeed.