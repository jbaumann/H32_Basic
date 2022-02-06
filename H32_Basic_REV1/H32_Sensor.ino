/*
 * The following functions implement the communication with the sensor.
 */

#include <Adafruit_AHTX0.h>

const uint8_t aht10_retries = 3;

/*
 * All of the following only because the implementation for an alternative address is missing.
 * A pull request is under way.
 */
#define AHTX0_I2CADDR_ALTERNATE 0x39

class My_AHTX0 : public Adafruit_AHTX0 {
public:
  bool begin(TwoWire *wire = &Wire, int32_t sensor_id = 0, uint8_t i2c_address = AHTX0_I2CADDR_DEFAULT);
};

bool My_AHTX0::begin(TwoWire *wire, int32_t sensor_id, uint8_t i2c_address) {
  delay(20); // 20 ms to power up

  if (i2c_dev) {
    delete i2c_dev; // remove old interface
  }

  i2c_dev = new Adafruit_I2CDevice(i2c_address, wire);

  if (!i2c_dev->begin()) {
    return false;
  }

  uint8_t cmd[3];

  cmd[0] = AHTX0_CMD_SOFTRESET;
  if (!i2c_dev->write(cmd, 1)) {
    return false;
  }
  delay(20);

  cmd[0] = AHTX0_CMD_CALIBRATE;
  cmd[1] = 0x08;
  cmd[2] = 0x00;
  if (!i2c_dev->write(cmd, 3)) {
    return false;
  }

  while (getStatus() & AHTX0_STATUS_BUSY) {
    delay(10);
  }
  if (!(getStatus() & AHTX0_STATUS_CALIBRATED)) {
    return false;
  }

  humidity_sensor = new Adafruit_AHTX0_Humidity(this);
  temp_sensor = new Adafruit_AHTX0_Temp(this);
  return true;
};
/*
 * End of the class definition
 */

My_AHTX0 aht;
//Adafruit_AHTX0 aht;

/*
 * Try to initialize the AHT10 sensor "aht10_retries" times before
 * giving up.
 */
bool init_sensor() {
  debug_println("AHT10 initialization");

  bool result = false;
  for(int i = 0; i < aht10_retries; i++) {
    result = aht.begin(&Wire, 0, AHTX0_I2CADDR_ALTERNATE);
    if(result) {
      debug_println("Found AHT sensor");
      break;
    }
    delay(100);
  }
  return result;
}

/*
 * Read the temperature from the sensor
 */
float get_temperature() {
  sensors_event_t temp;

  aht.getTemperatureSensor()->getEvent(&temp);

  debug_print("Temperature: ");
  debug_print(temp.temperature);
  debug_println(" degrees C");

  return temp.temperature;
}

/*
 * Read the humidity from the sensor
 */
float get_humidity() {
  sensors_event_t humidity;
  
  aht.getHumiditySensor()->getEvent(&humidity);

  debug_print("Humidity: ");
  debug_print(humidity.relative_humidity);
  debug_println(" % rH");

  return humidity.relative_humidity;
}
