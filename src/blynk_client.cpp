#include "blynk_client.h"
#include "config.h"
#if ENABLE_BLYNK
#define BLYNK_PRINT Serial
#define BLYNK_DEBUG
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/task.h>

namespace {

constexpr unsigned long BLYNK_DIAG_INTERVAL_MS = 5000;
constexpr unsigned long BLYNK_RETRY_INTERVAL_MS = 5000;
constexpr unsigned long BLYNK_CONNECT_TIMEOUT_MS = 1000;
constexpr unsigned long BLYNK_SKIP_LOG_INTERVAL_MS = 10000;
constexpr TickType_t BLYNK_TASK_DELAY_MS = pdMS_TO_TICKS(50);
constexpr TickType_t BLYNK_WIFI_DOWN_DELAY_MS = pdMS_TO_TICKS(250);
constexpr size_t BLYNK_NOTIFY_QUEUE_LEN = 4;
constexpr size_t BLYNK_NOTIFY_TEXT_LEN = 160;

struct BlynkTelemetrySample {
  int gas;
  float distance;
  uint8_t motion;
  int distanceLevel;
};

struct BlynkNotifyMessage {
  char text[BLYNK_NOTIFY_TEXT_LEN];
};

unsigned long lastBlynkDiagMs = 0;
unsigned long lastBlynkRetryMs = 0;
unsigned long lastBlynkSkipLogMs = 0;
unsigned long lastBlynkWifiDownMs = 0;
bool hasLoggedInvalidToken = false;
bool blynkConfigured = false;
volatile bool blynkCloudConnected = false;
volatile bool blynkTokenInvalid = false;
QueueHandle_t blynkTelemetryQueue = nullptr;
QueueHandle_t blynkNotifyQueue = nullptr;
TaskHandle_t blynkTaskHandle = nullptr;

const char* blynkStateLabel() {
  if (blynkCloudConnected) {
    return "connected";
  }
  if (blynkTokenInvalid) {
    return "token-invalid";
  }
  return "connecting";
}

void logBlynkStatus(const char* reason) {
  Serial.printf("Blynk: %s | wifi=%s | state=%s\n",
                reason,
                WiFi.status() == WL_CONNECTED ? "up" : "down",
                blynkStateLabel());
}

void syncBlynkState() {
  blynkCloudConnected = Blynk.connected();
  blynkTokenInvalid = Blynk.isTokenInvalid();
}

void ensureBlynkConfigured() {
  if (blynkConfigured) {
    return;
  }

  Serial.println("Blynk: Configuring New Blynk cloud...");
  Blynk.config(BLYNK_AUTH_TOKEN, "blynk.cloud", 80);
  blynkConfigured = true;
  logBlynkStatus("configured");
}

void sendLatestTelemetry() {
  if (!blynkTelemetryQueue || !Blynk.connected()) {
    return;
  }

  BlynkTelemetrySample sample;
  if (xQueueReceive(blynkTelemetryQueue, &sample, 0) != pdPASS) {
    return;
  }

  Blynk.virtualWrite(V0, sample.gas);
  if (sample.distance < 0) {
    Blynk.virtualWrite(V1, "--");
  } else {
    Blynk.virtualWrite(V1, (int)sample.distance);
  }
  Blynk.virtualWrite(V2, sample.motion ? 1 : 0);
  Blynk.virtualWrite(V4, sample.distanceLevel);
}

void sendQueuedNotifications() {
  if (!blynkNotifyQueue || !Blynk.connected()) {
    return;
  }

  BlynkNotifyMessage message;
  size_t sent = 0;
  while (sent < 2 && xQueueReceive(blynkNotifyQueue, &message, 0) == pdPASS) {
    Blynk.virtualWrite(V3, message.text);
    ++sent;
  }
}

void blynkTask(void*){
  ensureBlynkConfigured();
  Serial.println("Blynk: worker task started");

  if (WiFi.status() == WL_CONNECTED) {
    const bool connected = Blynk.connect(BLYNK_CONNECT_TIMEOUT_MS);
    syncBlynkState();
    Serial.printf("Blynk: initial connect attempt -> %s\n", connected ? "connected" : "not connected");
    if (!connected) {
      logBlynkStatus("initial connect pending");
    }
  }

  for (;;) {
    if (WiFi.status() != WL_CONNECTED) {
      const unsigned long now = millis();
      if (Blynk.connected()) {
        Blynk.disconnect();
        syncBlynkState();
      } else if (now - lastBlynkWifiDownMs >= BLYNK_DIAG_INTERVAL_MS) {
        lastBlynkWifiDownMs = now;
        logBlynkStatus("waiting for WiFi before Blynk");
      }
      vTaskDelay(BLYNK_WIFI_DOWN_DELAY_MS);
      continue;
    }

    Blynk.run();
    syncBlynkState();

    const unsigned long now = millis();
    if (blynkTokenInvalid) {
      if (!hasLoggedInvalidToken) {
        hasLoggedInvalidToken = true;
        logBlynkStatus("auth token rejected by Blynk");
      }
      vTaskDelay(BLYNK_TASK_DELAY_MS);
      continue;
    }

    if (!blynkCloudConnected) {
      if (now - lastBlynkDiagMs >= BLYNK_DIAG_INTERVAL_MS) {
        lastBlynkDiagMs = now;
        logBlynkStatus("waiting for cloud session");
      }

      if (now - lastBlynkRetryMs >= BLYNK_RETRY_INTERVAL_MS) {
        lastBlynkRetryMs = now;
        Serial.println("Blynk: forcing reconnect attempt");
        const bool connected = Blynk.connect(BLYNK_CONNECT_TIMEOUT_MS);
        syncBlynkState();
        Serial.printf("Blynk: reconnect attempt -> %s\n", connected ? "connected" : "not connected");
        if (!connected) {
          logBlynkStatus("reconnect pending");
        }
      }

      vTaskDelay(BLYNK_TASK_DELAY_MS);
      continue;
    }

    hasLoggedInvalidToken = false;
    sendQueuedNotifications();
    sendLatestTelemetry();
    vTaskDelay(BLYNK_TASK_DELAY_MS);
  }
}

}

BLYNK_CONNECTED() {
  blynkCloudConnected = true;
  blynkTokenInvalid = false;
  Serial.println("Blynk: cloud session established");
}

BLYNK_DISCONNECTED() {
  blynkCloudConnected = false;
  Serial.println("Blynk: cloud session dropped");
}

void blynkInit(){
  ensureBlynkConfigured();

  if (!blynkTelemetryQueue) {
    blynkTelemetryQueue = xQueueCreate(1, sizeof(BlynkTelemetrySample));
  }
  if (!blynkNotifyQueue) {
    blynkNotifyQueue = xQueueCreate(BLYNK_NOTIFY_QUEUE_LEN, sizeof(BlynkNotifyMessage));
  }

  if (!blynkTelemetryQueue || !blynkNotifyQueue) {
    Serial.println("Blynk: failed to create task queues");
    return;
  }

  if (!blynkTaskHandle) {
    BaseType_t created = xTaskCreatePinnedToCore(
      blynkTask,
      "blynkTask",
      6144,
      nullptr,
      2,
      &blynkTaskHandle,
      0
    );
    if (created != pdPASS) {
      blynkTaskHandle = nullptr;
      Serial.println("Blynk: failed to start task");
      return;
    }
  }

  logBlynkStatus("worker task ready");
}

void blynkPublishTelemetry(int gas, float distance, bool motion, int distanceLevel){
  if (!blynkTelemetryQueue) {
    Serial.println("Blynk: telemetry queue unavailable");
    return;
  }

  BlynkTelemetrySample sample = {
    gas,
    distance,
    static_cast<uint8_t>(motion ? 1 : 0),
    distanceLevel
  };

  xQueueOverwrite(blynkTelemetryQueue, &sample);

  if (!blynkCloudConnected) {
    const unsigned long now = millis();
    if (now - lastBlynkSkipLogMs >= BLYNK_SKIP_LOG_INTERVAL_MS) {
      lastBlynkSkipLogMs = now;
      logBlynkStatus("telemetry queued while offline");
    }
  }
}

void blynkNotify(const char* msg){
  if (!blynkNotifyQueue || !msg) {
    Serial.println("Blynk: notify queue unavailable");
    return;
  }

  BlynkNotifyMessage message = {};
  snprintf(message.text, sizeof(message.text), "%s", msg);
  if (xQueueSend(blynkNotifyQueue, &message, 0) == pdPASS) {
    return;
  }

  BlynkNotifyMessage dropped;
  (void)xQueueReceive(blynkNotifyQueue, &dropped, 0);
  if (xQueueSend(blynkNotifyQueue, &message, 0) != pdPASS) {
    Serial.println("Blynk: notify queue full, dropping message");
  }
}

void blynkRun(){
  // Blynk is handled by its dedicated FreeRTOS task.
}

#else
void blynkInit(){}
void blynkPublishTelemetry(int gas, float distance, bool motion, int distanceLevel){}
void blynkNotify(const char* msg){}
void blynkRun(){}
#endif
