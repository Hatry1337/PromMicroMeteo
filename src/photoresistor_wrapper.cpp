#include "photoresistor_wrapper.h"

Photoresistor::Photoresistor(void) {}

Photoresistor::~Photoresistor(void) {
  if (light_sensor) {
    delete light_sensor;
  }
}

bool Photoresistor::begin(uint8_t pin) {
  _pin = pin;

  delete light_sensor;
  light_sensor = new Photoresistor_Light(this);
  return true;
}

bool Photoresistor::getEvent(sensors_event_t *light) {
  uint32_t t = millis();

  _light = (float) 100 * (4095 - analogRead(_pin)) / 4095;

  // use helpers to fill in the events
  if (light) {
    fillLightEvent(light, t);
  }
  return true;
}

void Photoresistor::fillLightEvent(sensors_event_t *light, uint32_t timestamp) {
  memset(light, 0, sizeof(sensors_event_t));
  light->version = sizeof(sensors_event_t);
  light->sensor_id = _sensorid_light;
  light->type = SENSOR_TYPE_LIGHT;
  light->timestamp = timestamp;
  light->light = _light;
}

Adafruit_Sensor *Photoresistor::getLightSensor(void) {
  return light_sensor;
}

void Photoresistor_Light::getSensor(sensor_t *sensor) {
  /* Clear the sensor_t object */
  memset(sensor, 0, sizeof(sensor_t));

  /* Insert the sensor name in the fixed length char array */
  strncpy(sensor->name, "photoresistor", sizeof(sensor->name) - 1);
  sensor->name[sizeof(sensor->name) - 1] = 0;
  sensor->version = 1;
  sensor->sensor_id = _sensorID;
  sensor->type = SENSOR_TYPE_LIGHT;
  sensor->min_delay = 0;
  sensor->min_value = 0;
  sensor->max_value = 100;
  sensor->resolution = 0.5;
}

bool Photoresistor_Light::getEvent(sensors_event_t *event) {
  _thePhotores->getEvent(event);

  return true;
}
