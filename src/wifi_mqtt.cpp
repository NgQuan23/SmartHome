#include "wifi_mqtt.h"
#include "config.h"
#include "storage.h"
#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <WiFi.h>


static WiFiClient espClient;
static PubSubClient client(espClient);

void mqttCallback(char *topic, byte *payload, unsigned int length) {
  String msg;
  for (unsigned int i = 0; i < length; i++)
    msg += (char)payload[i];
  Serial.printf("MQTT in: %s -> %s\n", topic, msg.c_str());
}

void wifiInit() {
  Serial.printf("Connecting to WiFi '%s'...\n", WIFI_SSID);
  WiFi.disconnect(true);
  delay(100);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  Serial.println("\nWiFi connection initiated in background.");
  if (WiFi.status() == WL_CONNECTED) {
    Serial.printf("WiFi connected successfully. IP: %s\n",
                  WiFi.localIP().toString().c_str());
  } else {
    Serial.println("Continuing offline...");
  }
}

void mqttInit() {
  client.setServer(MQTT_SERVER, MQTT_PORT);
  client.setCallback(mqttCallback);
}

static bool mqttReconnect() {
  if (WiFi.status() != WL_CONNECTED)
    return false;
  Serial.print("Attempting MQTT connection...");
  String clientId = "ESP32-" + String((uint32_t)ESP.getEfuseMac());
  if (client.connect(clientId.c_str(), MQTT_USER, MQTT_PASSWORD)) {
    Serial.println("connected");
    client.subscribe(COMMAND_TOPIC);
    return true;
  }
  Serial.print("failed, rc=");
  Serial.println(client.state());
  return false;
}

void mqttLoop() {
  if (!client.connected()) {
    mqttReconnect();
  }
  client.loop();
}

bool mqttPublish(const char *topic, const char *payload) {
  if (!client.connected()) {
    if (!mqttReconnect())
      return false;
  }
  return client.publish(topic, payload);
}
