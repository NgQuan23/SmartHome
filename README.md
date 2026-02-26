# Smart Home System

## Project Overview

This is a Smart Home Security System built on ESP32. It uses various sensors to detect gas leaks, obstacles, and motion, and triggers corresponding actions to alert the user and mitigate risks. It is fully connected and controllable through multiple IoT cloud services.

## Current Features

The current implementation provides the following features and sensors:

*   **Sensors:**
    *   **Gas Sensor (MQ2):** Detects smoke and gas levels.
    *   **Ultrasonic Sensor (HC-SR04):** Measures distance to detect nearby objects/obstacles.
    *   **Motion Sensor (PIR):** Detects human motion.
*   **Actuators & Outputs:**
    *   **LCD Display (I2C):** Provides on-device status updates.
    *   **Buzzer:** Sounds alarms during critical situations.
    *   **Relays:** Controls external devices such as an Exhaust Fan, Main Power, and a Gas Valve.
*   **Alert Logic:**
    *   **Gas Alert:**
        *   Level 1: Low Leak (Safe status).
        *   Level 2: Warning Leak. Turns ON the Exaust Fan, sends Telegram and Blynk notifications.
        *   Level 3: Fire/Critical Leak. Sounds the Buzzer, Locks the Gas Valve, sends emergency notifications.
    *   **Distance/Obstacle Alert:**
        *   Level 1: Human detected.
        *   Level 2: Object near. Sounds the buzzer, sends warning notifications.
        *   Level 3: Critical proximity. Cuts power (via Main Power Relay), sounds buzzer, and sends emergency notifications.
    *   **Motion Alert:**
        *   Triggers buzzer and LCD alert when motion is detected.
*   **Connectivity & Integrations:**
    *   **WiFi:** Connects the ESP32 to the local network.
    *   **MQTT:** Publishes and subscribes to topics for local home automation integration.
    *   **Firebase Realtime Database:** Logs telemetry data.
    *   **Telegram Bot:** Sends direct alert messages to a specified Telegram Chat ID.
    *   **Blynk 2.0 (New Blynk IoT):** A fully customizable mobile dashboard to monitor sensor states and receive push notifications.
    *   **OTA (Over-The-Air) Updates:** Allows for wireless flashing of firmware.
    *   **Deep Sleep / Power Saving:** Enters deep sleep after a period of inactivity to conserve power.

## Configuration & Setup

### Setting up New Blynk IoT (Blynk 2.0)

This project has been updated to use the modern **Blynk IoT platform** (Blynk 2.0).
To configure your device:

1.  **Create a Template:**
    *   Go to [Blynk Console](https://blynk.cloud/) and log in.
    *   Navigate to **Templates** -> **New Template**.
    *   Name it (e.g., "Smart Home Security"), choose "ESP32" for Hardware and "WiFi" for Connection Type.
2.  **Define Datastreams:**
    Inside your template, go to the **Datastreams** tab and create the following Virtual Pins:
    *   **V0 (Integer):** Gas Sensor Value.
    *   **V1 (Double/Integer):** Distance (cm).
    *   **V2 (Integer):** PIR Motion State (0/1).
    *   **V3 (String):** Notifications / Alert Messages.
    *   **V4 (Integer):** Distance Alert Level.
3.  **Add a New Device:**
    *   Go to **Search** -> **Devices** -> **New Device**.
    *   Choose **From template** and select the template you just created. Give your device a name.
4.  **Copy Device Credentials:**
    *   Once your device is created, you will see a block of code with `#define BLYNK_TEMPLATE_ID`, `#define BLYNK_TEMPLATE_NAME`, and `#define BLYNK_AUTH_TOKEN`.
    *   Copy these three lines.
5.  **Update `config.h`:**
    *   Open `src/config.h` in your project folder.
    *   Locate the Blynk configuration section (around line 41).
    *   Replace the placeholder values with the credentials you copied:
        ```cpp
        /* IMPORTANT: Fill in your Blynk Template Info here before compiling */
        #define BLYNK_TEMPLATE_ID "TMPLxxxxxx"
        #define BLYNK_TEMPLATE_NAME "Device Name"
        #define BLYNK_AUTH_TOKEN "YourAuthToken"
        ```

### Customizing Other Services

Inside the `src/config.h` file, you can also configure your:
*   **WiFi Credentials** (`WIFI_SSID`, `WIFI_PASS`)
*   **MQTT Broker** settings
*   **Firebase** Database URL and authentication
*   **Telegram Bot Token** and Chat ID

---

*Powered by PlatformIO and ESP32.*