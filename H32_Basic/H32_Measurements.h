#ifndef H32_MEASUREMENTS_H
#define H32_MEASUREMENTS_H

#include "H32_Basic.h"

/*
 * Forward definitions for the needed functions
 */
double read_bat_voltage();
#ifdef H32_REV_3
double read_bat_percentage();
double read_bat_charge_rate();
#endif //H32_REV_3
double read_ext_voltage();
bool init_sensor();
float get_temperature();
float get_humidity();

/*
 * This class contains the data collected by the H32_Basic
 */
class H32_Measurements {
private:
  bool valid = false;
  bool initSuccess = false;
  double batV;
#ifdef H32_REV_3
  double batPercentage;
  double batChargeRate;
#endif //H32_REV_3
  double extV;
  double temperature;
  double humidity;
protected:
public:
  void readMeasurements() {
    if(!valid) {
      debug_println("Acquiring Measurements.");
      batV = read_bat_voltage();
      batPercentage = read_bat_percentage();
      batChargeRate = read_bat_charge_rate();
      extV = read_ext_voltage();
      if(init_sensor()) {
        temperature = (int)(get_temperature() * 100 + 0.5) / 100.0;
        humidity = (int)(get_humidity() * 100 + 0.5) / 100.0;
        initSuccess = true;
      } else {
        debug_println("AHT10 not found. Check your board.");
      }
      valid = true;
    }
  };
  double getBatV() { readMeasurements(); return batV;};
  double getBatPercentage() { readMeasurements(); return batPercentage;};
  double getBatChargeRate() { readMeasurements(); return batChargeRate;};
  double getExtV() { readMeasurements(); return extV; };
  double getTemperature() { readMeasurements(); return temperature; };
  double getHumidity() { readMeasurements(); return humidity; };
  void reset() { valid = false; };
  bool isValid() { return valid; };
  bool isInitSuccessful() { return initSuccess; };
};


#endif // H32_MEASUREMENTS_H
