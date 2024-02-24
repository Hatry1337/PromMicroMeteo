#include <Arduino.h>
#include <Adafruit_AHTX0.h>
#ifdef ESP32
#include <WiFi.h>
#include <AsyncTCP.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#endif
#include "ESPAsyncWebServer.h"
#include <OneWire.h>
#include "ds18b20_wrapper.h"
#include "photoresistor_wrapper.h"
#include "wifi_config.h"

Adafruit_AHTX0 aht;
DS18B20 ds;
Photoresistor pr;

AsyncWebServer server(80);

String location = "home";
sensors_event_t humidity, temp, temp_outside, light;

class RootRequestHandler : public AsyncWebHandler {
public:
  RootRequestHandler() {}
  virtual ~RootRequestHandler() {}

  bool canHandle(AsyncWebServerRequest *request){
    return request->url() == "/";
  }

  void handleRequest(AsyncWebServerRequest *request) {
    AsyncResponseStream *response = request->beginResponseStream("text/plain");
    
    response->print("PromMicroMeteo - prometheus-compatible meteorologic data collector.\n");
    response->print("Explore metrics at /metrics.\n");
    request->send(response);
  }
};

class MetricsRequestHandler : public AsyncWebHandler {
public:
  MetricsRequestHandler() {}
  virtual ~MetricsRequestHandler() {}

  bool canHandle(AsyncWebServerRequest *request){
    return request->url() == "/metrics";
  }

  void handleRequest(AsyncWebServerRequest *request) {
    AsyncResponseStream *response = request->beginResponseStream("text/plain");
    
    response->print("# HELP world_humidity Real world humidity data.\n");
    response->print("# TYPE world_humidity gauge\n");
    response->printf("world_humidity{location=\"%s\"} %f\n", location, humidity.relative_humidity);
    response->print("# HELP world_temperature Real world temperature data.\n");
    response->print("# TYPE world_temperature gauge\n");
    response->printf("world_temperature{location=\"%s\"} %f\n", location, temp.temperature);
    response->printf("world_temperature{location=\"outside\"} %f\n", temp_outside.temperature);
    response->print("# HELP world_light_intencity Real world ambient lightning data.\n");
    response->print("# TYPE world_light_intencity gauge\n");
    response->printf("world_light_intencity{location=\"%s\"} %f\n", location, light.light);
    
    request->send(response);
  }
};

class RequestLogger : public AsyncWebHandler {
public:
  RequestLogger() {}
  virtual ~RequestLogger() {}

  bool canHandle(AsyncWebServerRequest *request) {
    uint32_t ip = request->client()->getRemoteAddress();
    Serial.printf("[HTTP] %s at %s from %d.%d.%d.%d:%d\n", 
    request->methodToString(), request->url(), 
      (ip & 0x000000ff),
      (ip & 0x0000ff00) >> 8,
      (ip & 0x00ff0000) >> 16,
      (ip & 0xff000000) >> 24,
    request->client()->getRemotePort());
    return false;
  }
  
  void handleRequest(AsyncWebServerRequest *request) {
    AsyncResponseStream *response = request->beginResponseStream("text/plain");
    response->setCode(500);
    response->print("Something wrong with your configuration. This handler does not supposed to handle any requests.\n");
    request->send(response);
  }
};

void setup() {
  Serial.begin (9600);
  WiFi.begin(WIFI_STA_SSID, WIFI_STA_PASS);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.printf("Connecting to WiFi.. status=%d\n", WiFi.status());
    if(WiFi.status() == WL_DISCONNECTED) {
      Serial.println("WiFi disconnected. Reconnecting...");
      WiFi.reconnect();
      delay(500);
    }
  }

  Serial.print("Connected to the WiFi network. IP is ");Serial.println(WiFi.localIP().toString());

  if (aht.begin()) {
    Serial.println("Found AHT20");
  } else {
    Serial.println("Failed to find AHT20");
  }  

  if (ds.begin(new OneWire(10), 0x28)) {
    Serial.println("Found DS18B20");
  } else {
    Serial.println("Failed to find DS18B20");
  }  

  if (pr.begin(0)) {
    Serial.println("Found Photoresistor");
  } else {
    Serial.println("Failed to find Photoresistor");
  }

  server.addHandler(new RequestLogger());
  server.addHandler(new RootRequestHandler());
  server.addHandler(new MetricsRequestHandler());

  server.begin();
}

uint32_t lastMeasure = 0;

void loop() {
  uint32_t t = millis();
  if(t - lastMeasure > 1000) {
    aht.getEvent(&humidity, &temp);
    ds.getEvent(&temp_outside);
    pr.getEvent(&light);
    lastMeasure = t;
  }
}