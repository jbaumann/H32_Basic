#include <WiFiManager.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include "PCF85063A.h"
#include <PubSubClient.h>
#include <soc/soc.h>
#include <soc/rtc_cntl_reg.h>

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
 * bool f(char*, char*, double, double, double, double)
 */
bool thingspeak_call(char *api_key, char *api_additional, double temperature, double humidity, double bat_v, double ext_v);
bool iotplotter_call(char *api_key, char *api_additional, double temperature, double humidity, double bat_v, double ext_v);
bool (*api_calls[]) (char *, char *, double, double, double, double) = {
  thingspeak_call,
  iotplotter_call,
};

/*
 * This version value is stored in the config data and can be used to
 * determine whether the config data is valid
 */
const uint16_t h32_major_minor = H32_MAJOR << 8 | H32_MINOR;


/*
 * The Configuration data is saved to NVS (EEPROM) whenever the user changes something
 */

const char *h32_prefs_key = "h32_config";

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
