#include "ds18b20_wrapper.h"

/*!
 *    @brief  Instantiates a new AHTX0 class
 */
DS18B20::DS18B20(void) {}

DS18B20::~DS18B20(void) {
  if (temp_sensor) {
    delete temp_sensor;
  }
}

bool DS18B20::begin(OneWire *wire, uint8_t address) {
  delay(20); // 20 ms to power up

  _wireBus = wire;

  uint8_t newAddr;
  while(_wireBus->search(&newAddr)) {
    Serial.printf("[1wire-ds] device 0x%x on bus\n", newAddr);
    if(newAddr == address) {
      Serial.println("[1wire-ds] found device on bus");
      break;
    }
  }

  if(newAddr == 0 || newAddr != address) {
    Serial.println("failed to found device on 1-wire bus");
    return false;
  }

  delete temp_sensor;
  temp_sensor = new DS18B20_Temp(this);
  return true;
}

bool DS18B20::getEvent(sensors_event_t *temp) {
  // #TODO add address checks  

  uint32_t t = millis();

  _wireBus->reset();
  _wireBus->write(0xCC);
  _wireBus->write(0xBE);

  byte i;
  byte data[2];
  int16_t result;
  for (int i = 0; i < 2; i++) {
    data[i] = _wireBus->read();
  }

  result = (data[1]<<8) |data[0];
  int16_t whole_degree = (result & 0x07FF) >> 4; // cut out sign bits and shift
  _temperature = whole_degree +
  0.5 * ((data[0]&0x8)>>3) +
  0.25 * ((data[0]&0x4)>>2) +
  0.125 * ((data[0]&0x2)>>1) +
  0.0625 * (data[0]&0x1);
  if (data[1]&128) {
    _temperature *= -1;
  }

  _wireBus->reset();
  _wireBus->write(0xCC);
  _wireBus->write(0x44, 1);

  // use helpers to fill in the events
  if (temp) {
    fillTempEvent(temp, t);
  }
  return true;
}

void DS18B20::fillTempEvent(sensors_event_t *temp, uint32_t timestamp) {
  memset(temp, 0, sizeof(sensors_event_t));
  temp->version = sizeof(sensors_event_t);
  temp->sensor_id = _sensorid_temp;
  temp->type = SENSOR_TYPE_AMBIENT_TEMPERATURE;
  temp->timestamp = timestamp;
  temp->temperature = _temperature;
}

Adafruit_Sensor *DS18B20::getTemperatureSensor(void) {
  return temp_sensor;
}

void DS18B20_Temp::getSensor(sensor_t *sensor) {
  /* Clear the sensor_t object */
  memset(sensor, 0, sizeof(sensor_t));

  /* Insert the sensor name in the fixed length char array */
  strncpy(sensor->name, "DS18B20", sizeof(sensor->name) - 1);
  sensor->name[sizeof(sensor->name) - 1] = 0;
  sensor->version = 1;
  sensor->sensor_id = _sensorID;
  sensor->type = SENSOR_TYPE_AMBIENT_TEMPERATURE;
  sensor->min_delay = 0;
  sensor->min_value = -55;
  sensor->max_value = 125;
  sensor->resolution = 0.3;
}

bool DS18B20_Temp::getEvent(sensors_event_t *event) {
  _theDS18B20->getEvent(event);

  return true;
}
