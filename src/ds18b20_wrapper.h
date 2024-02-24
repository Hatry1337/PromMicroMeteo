#include "Adafruit_Sensor.h"
#include "OneWire.h"

class DS18B20;

class DS18B20_Temp : public Adafruit_Sensor {
public:
  DS18B20_Temp(DS18B20 *parent) { _theDS18B20 = parent; }

  bool getEvent(sensors_event_t *);
  void getSensor(sensor_t *);

private:
  int _sensorID = 0x2288;
  DS18B20 *_theDS18B20 = NULL;
};

class DS18B20 {
public:
  DS18B20();
  ~DS18B20();

  bool begin(OneWire *wire, uint8_t *address, uint16_t id = 0);
  bool getEvent(sensors_event_t *temp);
  uint8_t getStatus(void);
  Adafruit_Sensor *getTemperatureSensor(void);

protected:
  float _temperature; ///< Last reading's temperature (C)

  uint16_t _sensorid_temp;     ///< ID number for temperature

  uint8_t *_address; ///< address on bus

  OneWire *_wireBus = NULL;

  DS18B20_Temp *temp_sensor = NULL; ///< Temp sensor data object

private:
  friend class DS18B20_Temp;     ///< Gives access to private members to
                                        ///< Temp data object

  void fillTempEvent(sensors_event_t *temp, uint32_t timestamp);
};