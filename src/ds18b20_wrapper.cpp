#include "ds18b20_wrapper.h"
#include "utils.h"

uint16_t lastSensorId = 0;

/*!
 *    @brief  Instantiates a new AHTX0 class
 */
DS18B20::DS18B20(void) {}

DS18B20::~DS18B20(void) {
  if (temp_sensor) {
    delete temp_sensor;
  }
}

bool DS18B20::begin(OneWire *wire, uint8_t *address, uint16_t sensorId) {
  delay(20); // 20 ms to power up

  _wireBus = wire;
  _address = address;
  _sensorid_temp = sensorId;
  if(_sensorid_temp == 0) {
    if(lastSensorId != 0) {
      _sensorid_temp = lastSensorId;
    }
    lastSensorId++;
  }

  _wireBus->reset_search();

  uint8_t foundAddr[8];
  while(_wireBus->search(foundAddr)) {
    //Serial.printf("[1wire-ds] found device ");
    //printBytes(foundAddr, 8);
    //Serial.println("on 1-wire bus.");

    bool eq = true;
    for(uint8_t i = 0; i < 8; i++) {
      if(foundAddr[i] != _address[i]) {
        eq = false;
        break;
      }
    }

    if(!eq) {
      //Serial.println("[1wire-ds] addresses does not match. searching next...");
      continue;
    }

    //Serial.println("[1wire-ds] found matching address");

    if (OneWire::crc8(foundAddr, 7) != foundAddr[7]) {
      //Serial.print("[1wire-ds] invalid addr CRC!\n");
      return false;
    }

    if (foundAddr[0] != 0x28) {
      //Serial.print("[1wire-ds] device is not a DS18B20.\n");
      return false;
    }

    delete temp_sensor;
    temp_sensor = new DS18B20_Temp(this);
    return true;
  }

  //Serial.println("[1wire-ds] failed to found matching address");
  return false;
}

bool DS18B20::getEvent(sensors_event_t *temp) {
  uint32_t t = millis();

  _wireBus->reset();
  _wireBus->write(0x55); // send match rom command
  _wireBus->write_bytes(_address, 8); // send ds's address to match
  _wireBus->write(0x44, 1); // send perforom convertion command

  delay(750); // for 12bit resolution we need wait for 750ms to perform temperature conversion

  _wireBus->reset();
  _wireBus->write(0x55); // send match rom command
  _wireBus->write_bytes(_address, 8); // send ds's address to match
  _wireBus->write(0xBE); // read scratchpad

  byte i;
  byte data[2];
  int16_t result;
  for (int i = 0; i < 2; i++) {
    data[i] = _wireBus->read();
  }

  _wireBus->reset();

  //Serial.printf("[1wire-ds] ds18b20[%d] answer: ", _sensorid_temp);
  //printBytes(data, 2);
  //Serial.print("\n");
  
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
