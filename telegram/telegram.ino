/*
 Name:        echoBot.ino
 Created:     12/21/2017
 Author:      Stefano Ledda <shurillu@tiscalinet.it>
 Description: a simple example that check for incoming messages
              and reply the sender with the received message
*/
#include "CTBot.h"
CTBot myBot;

String ssid  = ""    ; // REPLACE mySSID WITH YOUR WIFI SSID
String pass  = ""; // REPLACE myPassword YOUR WIFI PASSWORD, IF ANY
String token = ""   ; // REPLACE myToken WITH YOUR TELEGRAM BOT TOKEN

void setup() {
  // initialize the Serial
  Serial.begin(115200);
  Serial.println("Starting TelegramBot...");

  // connect the ESP8266 to the desired access point
  myBot.wifiConnect(ssid, pass);

  // set the telegram bot token
  myBot.setTelegramToken(token);
  
  // check if all things are ok
  if (myBot.testConnection())
    Serial.println("\ntestConnection OK");
  else
    Serial.println("\ntestConnection NOK");

  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
  // a variable to store telegram message data
  TBMessage msg;

  // if there is an incoming message...
  if (myBot.getNewMessage(msg)) {
    Serial.print("received: ");
    Serial.println(msg.text);
    // ...forward it to the sender
    myBot.sendMessage(msg.sender.id, msg.text);
    //
    digitalWrite(BUILTIN_LED, msg.text.equals("on") ? LOW : HIGH);
  }
  // wait 500 milliseconds
  delay(500);
}
