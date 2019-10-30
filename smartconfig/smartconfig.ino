#include <ESP8266WiFi.h>;

void setup() {
  Serial.begin(115200);
  delay(100);

  WiFi.mode(WIFI_STA);
  delay(500);
}

void loop() {
  if (Serial.available() > 0) {
    while (Serial.available() > 0) {
      byte b = Serial.read();
      Serial.println(b);
    }
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("SmartConfig");
      WiFi.beginSmartConfig();
      while (!WiFi.smartConfigDone()) {
        delay(500);
        Serial.print("?");
      }
      while(WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
      }
      Serial.println("");
      WiFi.printDiag(Serial);
      Serial.println(WiFi.localIP());
      while (Serial.available() > 0) {
        byte b = Serial.read();
        Serial.println(b);
      }
    }
  }
}
