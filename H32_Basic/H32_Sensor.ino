/*
 * The following functions implement the communication with the sensor.
 */

#include "H32_Basic.h"
#include <AHTxx.h>

const uint8_t aht_retries = 3;

#ifdef H32_REV_3
#define H32_AHTX0_I2CADDR AHTXX_ADDRESS_X38
#define AHT_SENSOR_TYPE AHT2x_SENSOR
#else
#define H32_AHTX0_I2CADDR AHT10_ADDRESS_X39
#define AHT_SENSOR_TYPE AHT1x_SENSOR
#endif


AHTxx aht(H32_AHTX0_I2CADDR, AHT_SENSOR_TYPE);

/*
 * Try to initialize the AHTxx sensor "aht_retries" times before
 * giving up.
 */
bool init_sensor() {
  debug_println("AHTxx initialization");

  bool result = false;
  for(int i = 0; i < aht_retries; i++) {
    result = aht.begin();
    if(result) {
      debug_println("Found AHT sensor");
      break;
    }
    debug_println("AHT sensor not found");
//    aht.softReset();
    delay(100);
  }
  return result;
}

/*
 * Read the temperature from the sensor
 */
float get_temperature() {
  float temperature;

  temperature = aht.readTemperature();

  debug_print("Temperature: ");
  debug_print(temperature);
  debug_println(" degrees C");

  return temperature;
}

/*
 * Read the humidity from the sensor
 */
float get_humidity() {
  float humidity;
  
  humidity = aht.readHumidity(); ;

  debug_print("Humidity: ");
  debug_print(humidity);
  debug_println(" % rH");

  return humidity;
}
