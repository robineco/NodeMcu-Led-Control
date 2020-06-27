#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <FS.h>
#include <Wire.h>
#include <Adafruit_NeoPixel.h>
#include "AsyncJson.h"
#include "ArduinoJson.h"

#define PIN 14


const char* ssid = "";
const char* password = "";

AsyncWebServer server(80);

Adafruit_NeoPixel strip = Adafruit_NeoPixel(60, PIN, NEO_GRB + NEO_KHZ800);

int leds[6];
int numberOfLeds = 60;

int red = 0;
int green = 0;
int blue = 255;

int count = 0;
int brightness = 200;
int action = 0;
 
void setup(){
  Serial.begin(115200);
  if(!SPIFFS.begin()){
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }

  // Print ESP32 Local IP Address
  Serial.println(WiFi.localIP());

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.html", String());
  });
  server.on("/farbtastic.js", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/farbtastic.js", String());
  });
  server.on("/farbtastic.css", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/farbtastic.css", String());
  });
  server.on("/wheel.png", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/wheel.png", String());
  });
  server.on("/mask.png", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/mask.png", String());
  });
  server.on("/marker.png", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/marker.png", String());
  });
  server.on("/range.js", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/range.js", String());
  });

  //Turn Off
  server.on("/actions/0", HTTP_GET, [](AsyncWebServerRequest *request){
    action = 0;
    Serial.println("0");
    clearLeds();
    request->send(200, "text/plain", "");
  });

   //ON
  server.on("/actions/2", HTTP_GET, [](AsyncWebServerRequest *request){
    action = 2;
    Serial.println("2");
    clearLeds();
    oneColor();
    request->send(200, "text/plain", "");
  });

  //Restart
  server.on("/restart", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/plain", "");
    delay(500);
    ESP.restart();
  });


  //Spin
  server.on("/actions/1", HTTP_GET, [](AsyncWebServerRequest *request){
    action = 1;
    Serial.println("1");
    clearLeds();
    request->send(200, "text/plain", "");
  });

  //Ranibow
  server.on("/actions/4", HTTP_GET, [](AsyncWebServerRequest *request){
    action = 4;
    Serial.println("4");
    clearLeds();
    request->send(200, "text/plain", "");
  });

  //Change numberOfLeds
  server.on("/leds", HTTP_POST, [](AsyncWebServerRequest *request){
   }, NULL, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total){
      DynamicJsonBuffer jsonBuffer;
    JsonObject& root = jsonBuffer.parseObject((const char*)data);
    if (root.success()) {
      if (root.containsKey("number")) {
        numberOfLeds = root["number"];
        updateLeds();
        
      }
      request->send(200, "text/plain", "");
    } else {
      request->send(404, "text/plain", "");
    }
  });

  //Change color
  server.on("/color", HTTP_POST, [](AsyncWebServerRequest *request){
  }, NULL, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total){
    DynamicJsonBuffer jsonBuffer;  
    JsonObject& root = jsonBuffer.parseObject((const char*)data);
    if (root.success()) {
      if (root.containsKey("red")) {
        red = root["red"];
      }
      if (root.containsKey("green")) {
        green = root["green"];
      }
      if (root.containsKey("blue")) {
        blue = root["blue"];
      }
      oneColor();
      request->send(200, "text/plain", "");
    } else {
      request->send(404, "text/plain", "");
    }
  });

  //Change brightness
  server.on("/brightness", HTTP_POST, [](AsyncWebServerRequest *request){
  }, NULL, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total){
    DynamicJsonBuffer jsonBuffer;
    JsonObject& root = jsonBuffer.parseObject((const char*)data);
    if (root.success()) {
      if (root.containsKey("brightness")) {
        brightness = root["brightness"];
        Serial.println(brightness);
        strip.setBrightness(brightness);
        if (action == 2) {
          clearLeds();
          oneColor();
        }
      }
      request->send(200, "text/plain", "");
    } else {
      request->send(404, "text/plain", "");
    }
  });

  server.begin();

  strip.begin();
  strip.setBrightness(brightness);
  strip.show();

  for (int i = 0; i < 6; i++) {
    leds[i] = 59 - i; 
  }
  
}
 
void loop(){
  if (action == 1) {
    spin();
  } else if (action == 2) {
    
  } else if (action == 3) {
    
  } else if (action == 4) {
    rainbowCycle(20);
  } else {
    
  }
  
}

void updateLeds() {
  clearLeds();
  switch (action) {
    case 1:
      spin();
      break;
    case 2:
      oneColor();
      break;
    case 4:
      rainbowCycle(20);
      break;
  }
  
}

void clearLeds() {
  for (int i = 0; i < 60; i++) {
    strip.setPixelColor(i, strip.Color(0, 0, 0));
    strip.show();
  }
}

void spin() {
   if (count >= 120) {
    count = 0;
    red = random(0, 255);
    green = random(0, 255);
    blue = random(0, 255);
  }
  
  for (int i = 5; i >= 0; i--) {
    strip.setPixelColor(leds[i], strip.Color(0, 0, 0));
    leds[i] -= 1;
    if (leds[i] <= 0) {
      leds[i] = 59;
    }
    strip.setPixelColor(leds[i], strip.Color(red, green, blue));
  }
  strip.show();
    delay(20);
    count++;
}

void oneColor() {
  for (int i = 0; i < numberOfLeds; i++) {
    strip.setPixelColor(i, strip.Color(red, green, blue));
    strip.show();
  }
}

void rainbowCycle(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256*5; j++) { // 5 cycles of all colors on wheel
    for(i=0; i< strip.numPixels(); i++) {
      if (action != 4) {
        break;
      }
      strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}
  
