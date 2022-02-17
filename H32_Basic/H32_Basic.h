#ifndef H32_BASIC_H
#define H32_BASIC_H

#include <unordered_map>

using namespace std;

#include <WiFiManager.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <soc/soc.h>
#include <soc/rtc_cntl_reg.h>

#include "PCF85063A.h"
/* 
 * The following macros allow us to enable/disable debugging without runtime overhead
 */
#define SERIAL_SPEED 115200
#ifdef H32_DEBUG
#define debug_init() do { Serial.begin(SERIAL_SPEED); while (!Serial) {;} } while (0)
#define debug_print(...) do { Serial.print(__VA_ARGS__); } while (0)
#define debug_println(...) do { Serial.println(__VA_ARGS__); } while (0)
#else
#define debug_init()
#define debug_print(...)
#define debug_println(...)
#endif

/*
 * The WiFiManager is used for all communication with the user via the web portal
 */
WiFiManager wm;
const uint8_t SSID_LENGTH = 33;
const uint8_t NAME_LENGTH = 50;
const uint8_t TOPIC_LENGTH = 100;  // in theory 32.767 characters
const uint8_t U32_LENGTH = 10;
const uint8_t U16_LENGTH = 5;
const uint8_t U8_LENGTH = 3;
const uint8_t DOUBLE_LENGTH = 10;

/*
 * Forward definitions for the following class
 */
double read_bat_voltage();
double read_ext_voltage();
bool init_sensor();
float get_temperature();
float get_humidity();

/*
 * This class contains the data collected by the H32_Basic
 */
struct H32_Measurements {
private:
  bool valid = false;
  bool initSuccess = false;
  double batV;
  double extV;
  double temperature;
  double humidity;
protected:
public:
  void readMeasurements() {
    if(!valid) {
      debug_println("Acquiring Measurements.");
      batV = read_bat_voltage();
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
  double getExtV() { readMeasurements(); return extV; };
  double getTemperature() { readMeasurements(); return temperature; };
  double getHumidity() { readMeasurements(); return humidity; };
  void reset() { valid = false; };
  bool isValid() { return valid; };
  bool isInitSuccessful() { return initSuccess; };
};

/*
 * One of the Parameter entries in the WiFiManager is a combination of dropdown and
 * normal input field. This has to be handwritten and the following info is used
 * for that. The APIType is also used in the function table that contains the 
 * communication with external services (API Calls)
 */
enum APIType : int8_t {
  none = 0,
  thingspeak = 1,
  iotplotter = 2,
};
const char *apitype_names[] = {
  "---",
  "Thingspeak",
  "IOT Plotter",
};
const int apitype_num = sizeof(apitype_names)/sizeof(char *);

/*
 * This table contains the functions for communication with external APIs.
 * The function signature is 
 * bool f(char*, char*, H32_Measurements &, unordered_map<char *, double> &)
 */
bool thingspeak_call(char *api_key, char *api_additional, H32_Measurements &measurements, unordered_map<char *, double> &additional_data);
bool iotplotter_call(char *api_key, char *api_additional, H32_Measurements &measurements, unordered_map<char *, double> &additional_data);
bool (*api_calls[]) (char *, char *, H32_Measurements &, unordered_map<char *, double> &) = {
  thingspeak_call,
  iotplotter_call,
};

/*
 * The size of json document buffers
 */
const uint16_t json_doc_size = 1024;

/*
 * the signature of our interrupt function
 */
void IRAM_ATTR button_interrupt_function();


/*
 * This version value is stored in the config data and can be used to
 * determine whether the config data is valid
 */
const uint16_t h32_major_minor = H32_MAJOR << 8 | H32_MINOR;


/*
 * The Configuration data is saved to LittleFS as a json file whenever
 * the user changes something.
 */

const char *h32_prefs_key = "h32_config";
const char *h32_prefs_dir = "/h32_config";
const char *h32_prefs_path = "/h32_config/h32_config.json";

struct H32_Config_t {
  uint16_t version = h32_major_minor;
  uint16_t timeout = 20;
  char name[SSID_LENGTH+1];
  int8_t led_pin = 2;
  int8_t trigger_pin = 0;
  struct {
    uint32_t sleeptime = 10;
    double factor = 1;
    double limit = 1;
  } rtc;
  struct {
    APIType type;
    char key[NAME_LENGTH+1];
    char additional[NAME_LENGTH+1];
  } api;
  struct {
    double coefficient = 1.0;
    double constant = 0.0;
    int8_t pin = 33;
    int8_t activation = 0;
  } bat_v;
  struct {
    double coefficient = 1.0;
    double constant = 0.0;
    int8_t pin = 34;
  } ext_v;
  struct {
    char server[NAME_LENGTH+1];
    uint16_t port = 1883;
    char topic[TOPIC_LENGTH+1];
    char user[NAME_LENGTH+1];
    char passwd[NAME_LENGTH+1];
  } mqtt;
  struct {
    char server[NAME_LENGTH+1] {"pool.ntp.org"};
    int8_t daylightOffset_h = 1;
    int8_t gmtOffset_h = 1;
  } ntp;
} h32_config;

#endif // H32_BASIC_H
