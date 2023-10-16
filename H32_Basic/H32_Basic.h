#ifndef H32_BASIC_H
#define H32_BASIC_H

/*
 * The following definition is used to distinguish between the different revisions of the H32.
 * If H32_REV_3 is defined we compile for the revision 3 of the H32 board
 */
#define H32_REV_3

#ifdef H32_REV_3
#pragma message "Compiling for H32 revision 3"
#else
#pragma message "Compiling for H32 revision 1 or 2"
#endif

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

#ifdef H32_REV_3
#include <SparkFun_MAX1704x_Fuel_Gauge_Arduino_Library.h>
#endif //H32_REV_3

#include "PCF85063A.h"

#include "H32_Measurements.h"

const uint8_t SSID_LENGTH = 33;
const uint8_t NAME_LENGTH = 50;
const uint8_t TOPIC_LENGTH = 100;  // in theory 32.767 characters
const uint8_t IP_ADDR_LENGTH = 16;
const uint8_t U32_LENGTH = 10;
const uint8_t U16_LENGTH = 5;
const uint8_t U8_LENGTH = 3;
const uint8_t DOUBLE_LENGTH = 10;

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

typedef struct H32_Config {
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
  struct {
    char ip_address[IP_ADDR_LENGTH+1] {""};
    char gateway[IP_ADDR_LENGTH+1] {""};
    char subnet[IP_ADDR_LENGTH+1] {""};
    char dns[IP_ADDR_LENGTH+1] {"8.8.8.8"};
  } static_conf;
} H32_Config;

#endif // H32_BASIC_H
