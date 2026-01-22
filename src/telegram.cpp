#include "telegram.h"
#include "config.h"
#include <WiFiClientSecure.h>
#include <HTTPClient.h>

// simple URL-encode helper
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

void telegramSend(const String &message){
  if (!ENABLE_TELEGRAM) return;
  if (WiFi.status() != WL_CONNECTED) return;
  WiFiClientSecure client;
  client.setInsecure(); 
  HTTPClient https;
  String url = String("https://api.telegram.org/bot") + TG_BOT_TOKEN + "/sendMessage";
  https.begin(client, url);
  https.addHeader("Content-Type", "application/x-www-form-urlencoded");
  String post = "chat_id=" + String(TG_CHAT_ID) + "&text=" + urlencode(message);
  int httpCode = https.POST(post);
  if (httpCode > 0) {
    Serial.printf("Telegram sent, code=%d\n", httpCode);
  } else {
    Serial.printf("Telegram failed, err=%s\n", https.errorToString(httpCode).c_str());
  }
  https.end();
}
