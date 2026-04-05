#include "telegram.h"
#include "config.h"
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

String urlencode(const String &str){
  String encoded = "";
  char c;
  for (size_t i = 0; i < str.length(); ++i) {
    c = str[i];
    if (('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || ('0' <= c && c <= '9') || c=='-' || c=='_' || c=='.' || c=='~'){
      encoded += c;
    } else if (c == ' ') encoded += '+';
    else {
      char buf[5];
      sprintf(buf, "%%%.2X", (unsigned char)c);
      encoded += buf;
    }
  }
  return encoded;
}

bool telegramSend(const String &message){
  if (!ENABLE_TELEGRAM) return false;
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Telegram skipped: WiFi not connected");
    return false;
  }
  if (String(TG_BOT_TOKEN).length() == 0 || String(TG_CHAT_ID).length() == 0) {
    Serial.println("Telegram skipped: missing bot token or chat id");
    return false;
  }

  WiFiClientSecure client;
  client.setInsecure();
  client.setTimeout(5000);
  HTTPClient https;
  https.setTimeout(5000);
  String url = String("https://api.telegram.org/bot") + String(TG_BOT_TOKEN) + "/sendMessage";
  if (!https.begin(client, url)) {
    Serial.println("Telegram failed: unable to start HTTPS client");
    return false;
  }
  https.addHeader("Content-Type", "application/x-www-form-urlencoded");
  String post = "chat_id=" + String(TG_CHAT_ID) + "&text=" + urlencode(message);
  int httpCode = https.POST(post);
  String response;
  if (httpCode > 0) {
    response = https.getString();
  }

  bool ok = false;
  if (httpCode > 0) {
    DynamicJsonDocument doc(2048);
    DeserializationError err = deserializeJson(doc, response);
    if (httpCode == HTTP_CODE_OK && !err && doc["ok"] == true) {
      long messageId = doc["result"]["message_id"] | 0L;
      Serial.printf("Telegram sent, code=%d, message_id=%ld\n", httpCode, messageId);
      ok = true;
    } else if (err) {
      Serial.printf("Telegram failed, code=%d, invalid JSON response: %s\n", httpCode, response.c_str());
      if (httpCode == HTTP_CODE_OK) ok = true; // Still mark true if code 200 but failed parsing
    } else {
      const char *description = doc["description"] | "unknown Telegram API error";
      Serial.printf("Telegram failed, code=%d, api_error=%s\n", httpCode, description);
    }
  } else {
    Serial.printf("Telegram failed, err=%s\n", https.errorToString(httpCode).c_str());
  }
  https.end();
  return ok;
}
