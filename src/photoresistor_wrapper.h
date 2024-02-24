#include "Adafruit_Sensor.h"
#include "OneWire.h"

class Photoresistor;

class Photoresistor_Light : public Adafruit_Sensor {
public:
  Photoresistor_Light(Photoresistor *parent) { _thePhotores = parent; }

  bool getEvent(sensors_event_t *);
  void getSensor(sensor_t *);

private:
  int _sensorID = 0x1337;
  Photoresistor *_thePhotores = NULL;
};

class Photoresistor {
public:
  Photoresistor();
  ~Photoresistor();

  bool begin(uint8_t pin);

  bool getEvent(sensors_event_t *light);
  uint8_t getStatus(void);
  Adafruit_Sensor *getLightSensor(void);

protected:
  float _light;

  uint16_t _sensorid_light;

  uint8_t _pin;

  Photoresistor_Light *light_sensor = NULL;

private:
  friend class Photoresistor_Light;     ///< Gives access to private members to
                                        ///< Temp data object

  void fillLightEvent(sensors_event_t *light, uint32_t timestamp);
};