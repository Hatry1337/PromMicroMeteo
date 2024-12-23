#include <Arduino.h>
#include <Adafruit_AHTX0.h>
#include <Adafruit_BME280.h>
#include <Adafruit_HMC5883_U.h>
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

#define WIRE Wire

Adafruit_AHTX0 aht;
Adafruit_BME280 bme;
//Adafruit_HMC5883_Unified mag = Adafruit_HMC5883_Unified(12345);
DS18B20 ds;
DS18B20 ds2;
Photoresistor pr;
OneWire wireBus(6); // select one-wire bus gpio pin 
AsyncWebServer server(80);

String location = "home";
sensors_event_t humidity, temp, temp_outside, temp_test, light, magnetic;
float bme_humidity, bme_temp, bme_pressure;

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
    response->printf("world_humidity{location=\"%s\", sensor_type=\"aht10\"} %f\n", location, humidity.relative_humidity);
//    response->printf("world_humidity{location=\"home_bme\", sensor_type=\"bme280\"} %f\n", location, bme_humidity);

    response->print("# HELP world_temperature Real world temperature data.\n");
    response->print("# TYPE world_temperature gauge\n");
    response->printf("world_temperature{location=\"%s\", sensor_type=\"aht10\"} %f\n", location, temp.temperature);
    response->printf("world_temperature{location=\"outside\", sensor_type=\"ds18b20\"} %f\n", temp_outside.temperature);
//    response->printf("world_temperature{location=\"test\", sensor_type=\"ds18b20\"} %f\n", temp_test.temperature);
    response->printf("world_temperature{location=\"home_bme\", sensor_type=\"bme280\"} %f\n", bme_temp);

    response->print("# HELP world_light_intencity Real world ambient lightning data.\n");
    response->print("# TYPE world_light_intencity gauge\n");
    response->printf("world_light_intencity{location=\"%s\", sensor_type=\"photoresistor\"} %f\n", location, light.light);

    response->print("# HELP world_atmospheric_pressure Real world atmospheric pressure data.\n");
    response->print("# TYPE world_atmospheric_pressure gauge\n");
    response->printf("world_atmospheric_pressure{location=\"%s\", sensor_type=\"bme280\"} %f\n", location, bme_pressure);

    response->print("# HELP world_magnetic_field Real world magnetic field data.\n");
    response->print("# TYPE world_magnetic_field gauge\n");
    response->printf("world_magnetic_field{location=\"%s\", sensor_type=\"hmc5883\", axis=\"x\"} %f\n", location, magnetic.magnetic.x);
    response->printf("world_magnetic_field{location=\"%s\", sensor_type=\"hmc5883\", axis=\"y\"} %f\n", location, magnetic.magnetic.y);
    response->printf("world_magnetic_field{location=\"%s\", sensor_type=\"hmc5883\", axis=\"z\"} %f\n", location, magnetic.magnetic.z);


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

uint8_t dsAddr[8] = { 0x28, 0xc7, 0x68, 0xba, 0x96, 0x23, 0xb, 0xec };
uint8_t ds2Addr[8] = { 0x28, 0x5e, 0xb6, 0x91, 0x96, 0x23, 0xb, 0xd3 };
bool setupCompleted = false;

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

  if (aht.begin(&Wire)) {
    Serial.println("Found AHT20");
  } else {
    Serial.println("Failed to find AHT20");
  }  

  if (bme.begin((uint8_t)0x76, &Wire)) {
    Serial.println("Found BME280");
  } else {
    Serial.println("Failed to find BME280");
    Serial.print("SensorID was: 0x"); Serial.println(bme.sensorID(),16);
  }

  //if (mag.begin()) {
  //  Serial.println("Found HMC5883");
  //} else {
  //  Serial.println("Failed to find HMC5883");
  //}

  if (ds.begin(&wireBus, dsAddr)) {
    Serial.println("Found DS18B20");
  } else {
    Serial.println("Failed to find DS18B20");
  }

  // if (ds2.begin(&wireBus, ds2Addr)) {
  //  Serial.println("Found DS18B20 2");
  // } else {
  //  Serial.println("Failed to find DS18B20 2");
  // }  

  if (pr.begin(0)) {
    Serial.println("Found Photoresistor");
  } else {
    Serial.println("Failed to find Photoresistor");
  }

  server.addHandler(new RequestLogger());
  server.addHandler(new RootRequestHandler());
  server.addHandler(new MetricsRequestHandler());

  server.begin();
  setupCompleted = true;
}

uint32_t lastMeasure = 0;

void loop() {
  if(!setupCompleted) return;
  
  uint32_t t = millis();
  if(t - lastMeasure > 1000) {
    aht.getEvent(&humidity, &temp);
    
    bme_temp = bme.readTemperature();
    bme_humidity = bme.readHumidity();
    bme_pressure = bme.readPressure();

    pr.getEvent(&light);
    ds.getEvent(&temp_outside);
    //ds2.getEvent(&temp_test);
    //mag.getEvent(&magnetic);
    lastMeasure = t;
  }
}