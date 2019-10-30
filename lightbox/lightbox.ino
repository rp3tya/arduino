/**
 * MIT License
 *
 * Copyright (c) 2017 David McCann
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#define FASTLED_ESP8266_RAW_PIN_ORDER
#include <FastLED.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <EEPROM.h>
#include "msgpack.h"

WiFiUDP udp;

constexpr int16_t FIRMWARE_VERSION = 3;

constexpr int16_t NUM_ROWS = 1;
constexpr int16_t NUM_COLUMNS = 9;
constexpr int16_t NUM_LEDS = NUM_ROWS * NUM_COLUMNS;

constexpr uint8_t DATA_PIN = 5;
constexpr uint8_t RESET_PIN = 4;
constexpr uint8_t PWM_PIN = 2;

int16_t brightness = 255;
CRGB leds[NUM_LEDS];

void reset() {
    Serial.println("Reset Wifi Settings");
    for (int i = 0; i < 512; ++i) {
        EEPROM.write(i, 0);
    }
    EEPROM.commit();
    delay(1000);
    ESP.restart();
}

void setup() {
    Serial.begin(9600);
    FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
    pinMode(RESET_PIN, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(RESET_PIN), reset, FALLING);
    EEPROM.begin(512);
    if (EEPROM.read(0)) {
        Serial.println("Connect to saved SSID");
        connectToWifi();
    }
    else {
        Serial.println("Create WiFi AP");
        WiFi.mode(WIFI_AP);
        WiFi.softAP("Lightbox", "lightbox12345");
        FastLED.showColor(CRGB(0, 0, 255));
    }
    udp.begin(7777);
}

void connectToWifi() {
    FastLED.showColor(CRGB(255, 0, 0));
    uint8_t ssidSize = EEPROM.read(1);
    Serial.println(ssidSize);
    String ssid;
    for (int i = 0; i < ssidSize; ++i) {
        ssid += (char)EEPROM.read(2 + i);
    }
    uint8_t passSize = EEPROM.read(2 + ssidSize);
    Serial.println(passSize);
    String pass;
    for (int i = 0; i < passSize; ++i) {
        pass += (char)EEPROM.read(3 + ssidSize + i);
    }
    Serial.print(ssid);
    Serial.print("; ");
    Serial.print(pass);
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid.c_str(), pass.c_str());
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    FastLED.showColor(CRGB(0, 255, 0));
}

void loop() {
    if (udp.parsePacket()) {
        parseMessage();
    }
}

void handleFrame() {
    static uint16_t b;
    uint16_t br, bg, bb = 0;
    //
    Serial.println("Handle fram");
    uint8_t red, green, blue;
    for (int16_t i = 0; i < NUM_LEDS; ++i) {
        msgpck_read_integer(&udp, &red, sizeof(uint8_t));
        msgpck_read_integer(&udp, &green, sizeof(uint8_t));
        msgpck_read_integer(&udp, &blue, sizeof(uint8_t));
        leds[i].setRGB(red, green, blue);
        //
        br = red;
        bg = green;
        bb = blue;
    }
    FastLED.show();
    //
    b = 1024 - (0.21*br + 0.72*bg + 0.07*bb);
    analogWrite(PWM_PIN, b);
    Serial.print(br);Serial.print(" ");
    Serial.print(bg);Serial.print(" ");
    Serial.print(bb);Serial.print(" ");
    Serial.println(b);
}

void handleBrightness() {
    msgpck_read_integer(&udp, reinterpret_cast<byte*>(&brightness), sizeof(uint8_t));
    FastLED.setBrightness(brightness);
    FastLED.show();
    Serial.println("Change brightness");
    //
    analogWrite(PWM_PIN, 4*brightness);
    analogWrite(D0, 4*brightness);
}

void handleWiFi() {
    char ssid[64] = { 0 }, pass[64] = { 0 };
    uint32_t ssidSize, passSize;
    msgpck_read_string(&udp, ssid, 64, &ssidSize);
    msgpck_read_string(&udp, pass, 64, &passSize);

    Serial.println("Update WiFi Settings");
    Serial.println(ssid);
    Serial.println(pass);

    EEPROM.write(0, 1);
    EEPROM.write(1, static_cast<uint8_t>(ssidSize));
    for (int i = 0; i < ssidSize; ++i) {
        EEPROM.write(2 + i, ssid[i]);
    }
    EEPROM.write(2 + ssidSize, static_cast<uint8_t>(passSize));
    for (int i = 0; i < passSize; ++i) {
        EEPROM.write(3 + ssidSize + i, pass[i]);
    }
    EEPROM.commit();
    delay(1000);
    ESP.restart();
}

void handleDiscovery() {
    Serial.println("Received discovery request");
    IPAddress ip = udp.remoteIP();
    uint16_t port = udp.remotePort();
    udp.beginPacket(ip, port);
    msgpck_write_integer(&udp, FIRMWARE_VERSION);
    msgpck_write_integer(&udp, brightness);
    msgpck_write_integer(&udp, NUM_ROWS);
    msgpck_write_integer(&udp, NUM_COLUMNS);
    udp.endPacket();
}

void handlePing() {
    IPAddress ip = udp.remoteIP();
    uint16_t port = udp.remotePort();
    udp.beginPacket(ip, port);
    udp.write(1);
    udp.endPacket();
}

constexpr uint8_t MSG_FRAME = 1;
constexpr uint8_t MSG_BRIGHTNESS = 2;
constexpr uint8_t MSG_WIFI = 3;
constexpr uint8_t MSG_DISCOVERY = 4;
constexpr uint8_t MSG_PING = 5;

void parseMessage() {
    uint8_t method;
    if (msgpck_read_integer(&udp, &method, sizeof(uint8_t))) {
        switch (method) {
        case MSG_FRAME:
            handleFrame();
            break;
        case MSG_BRIGHTNESS:
            handleBrightness();
            break;
        case MSG_WIFI:
            handleWiFi();
            break;
        case MSG_DISCOVERY:
            handleDiscovery();
            break;
        case MSG_PING:
            handlePing();
            break;
        }
    }
}
