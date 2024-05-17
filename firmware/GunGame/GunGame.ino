#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <EncButton.h>
#include <WebSocketsServer.h>
#include <Wire.h>

#include "TCS3472.h"
#include "led.h"
#include "tmr.h"
#define VIB_PIN 12
#define TRIG_PIN 2
#define LED_PIN 15

WebSocketsServer ws(81, "", "Arduino");
TCS3472 rgb;
Button btn(TRIG_PIN);
Led led(LED_PIN);


void setup() {
  // WIFI
  Serial.begin(115200);

  const char* ssid = "Tak123";
  const char* password =  "12345678";
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
     led.toggle();
     delay(1000);
     Serial.println("Connecting..");
   }
  led.off();
  Serial.println();
  Serial.println(WiFi.localIP());

  // WS
   ws.begin();
   ws.onEvent([](uint8_t num, WStype_t type, uint8_t* data, size_t len) {
     switch (type) {
       case WStype_CONNECTED:
         led.on();
         Serial.println("Подключено куда-то");
         break;

       default:
         break;
     }
   });

  // RGB
  Wire.begin();
   if (!rgb.begin(&Wire)) {   
     Serial.println("sensor error");
    //  while (1) {// Может из-за этого не подключается
    //    led.toggle();
    //    delay(100);
    //  }
   }
  rgb.setTime(tcs_time_t::T24);
  rgb.setGain(tcs_gain_t::X16);
  pinMode(VIB_PIN,OUTPUT);
  Serial.println("setup end");
}

void sendColor(bool shot) {
  tcs_color_t color = rgb.getRaw();
  String s;
  s += color.r;
  s += ',';
  s += color.g;
  s += ',';
  s += color.b;
  s += ',';
  s += shot;
  // Serial.println(s);
  ws.broadcastTXT((uint8_t*)s.c_str(), s.length());
}

void loop() {
  ws.loop();
  if (!ws.connectedClients()) {
    static Tmr tmr(250);
    if (tmr) led.toggle();
  }

  btn.tick();
  if (btn.press()) {
    sendColor(true);
    Serial.println("work");
    digitalWrite(VIB_PIN, 1);
    delay(600);
    digitalWrite(VIB_PIN, 0);
    }
  if (btn.holding()) {
    
    static Tmr tmr(100);
    if (tmr) sendColor(false);
  }
}
