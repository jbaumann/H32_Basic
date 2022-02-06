/*
 * H32 Basic Program
 * This is a firmware for the H32 board by Burgduino providing the following functionality:
 * OTA Updates
 * Portal for entering credentials
 * Portal allows to enter additional configuration data
 * GPIO0 leads to config portal after start (i.e. after LED is turned on)
 * A second GPIO pin is configurable as additional trigger pin
 * Store Data in NVS
 * Configurable LED pin
 * Page that allows scanning for I2C devices
 * Page showing the current measurements
 * Thingspeak communication
 * IOTPlotter Communication
 * Reading the sensor
 * Portal allows to set the RTC to NTP time
 * Failed Connection Counter stored in RTC memory
 * Dynamic, configurable increase of sleep time when WiFi is not reachable
 * MQTT
 *
 * The following third-party libraries are used in this sketch:
 *   Adafruit_AHTX0 by Adafruit
 *   WiFiManager by tzapu
 *   Thingspeak by Mathworks
 *   Arduino Client for MQTT by Nick O’Leary
 *
 * These can be installed using the library manager of the Arduino IDE (or downloaded from Github)
 * An additional library for the PCF85063 by Jaakko Salo has been modified to some extent and is
 * directly included.
 *
 * Author: Joachim Baumann
 */

/*
 * The following three values represent the current version of the firmware
 */
const uint8_t H32_MAJOR = 1;
const uint8_t H32_MINOR = 12;
const uint8_t H32_PATCH = 2;

/*
 * The following values can be adjusted
 */

// H32_DEBUG has to be set to allow debug messages over serial. If not needed, simply comment it.
#define H32_DEBUG

// This is the password for the captive portal created when no WiFi credentials are stored
const char* ap_passwd = "sokrates";

// This is the fallback button that allows to jump into the configuration portal
const int button = 0;

// This is the hardware pin that allows the H32 to turn itself off completely
const int DONE = 13;


// The header file in turn includes all needed header files and defines everything needed
#include "H32_Basic.h"


void setup() {
  // Turn off any alarm in RTC
  PCF85063A_Regs rtc_results = RTC_stop_and_check();

  // The ap_name is used for the access point name, the device name
  // and the title of the config page
  create_AP_Name(h32_config.name);
  debug_println(h32_config.name);

  // Read the configuration from NVS
  read_config();

  // Configure the button interrupts
  attachInterrupt(digitalPinToInterrupt(button), button_interrupt_function, FALLING);
  if(h32_config.trigger_pin > 0) {
    attachInterrupt(digitalPinToInterrupt(h32_config.trigger_pin), button_interrupt_function, FALLING);  
  }

  // Init LED and turn it on, if configured
  led_init();
  led_on();

  // Init debug printing and print a first newline
  debug_init();
  debug_println();

  // The rtc_results allow to differentiate between a "normal" reset and an RTC-triggered reset
  if(rtc_results != 0) {
    debug_print("Countdown or Alarm has been triggered: ");
    debug_println(rtc_results);
  }

  // We initialize the WiFiManager that checks for stored credentials. If none are available,
  // a captive portal is opened. Otherwise it tries to connect to the network.
  init_WiFiManager();

  // Some visual feedback
  led_toggle();

  // If the button has been pressed for longer than a second, we jump to the configuration portal
  if(button_is_pressed()){
    return; // jump to loop()
  }

  // If the WiFiManager was able to connect us to the network, then we send our data
  // Otherwise, we increment the backoff counter in the RTC ram
  if(WiFi.isConnected()){
    // Reset failed connections counter
    RTC_set_RAM(0);
    // Collect data and send it to the chosen channels
    read_and_send_data();
  } else {
    RTC_increment_RAM();
  }

  // If the button has been pressed for longer than a second, we jump to the configuration portal
  if(button_is_pressed()){
    return; // jump to loop()
  }

  // calculate backoff time if needed by:
  // calculated factor = (configured backoff factor) ^ (failed connects)
  // if the factor is larger than the limit, the limit is used instead
  // sleeptime = (configured sleeptime) * factor
  uint8_t failed_conns = RTC_get_RAM();
  uint32_t sleeptime = h32_config.rtc.sleeptime;
  double factor = pow(h32_config.rtc.factor, failed_conns);
  factor = factor > h32_config.rtc.limit ? h32_config.rtc.limit : factor;
  sleeptime *= factor;

  // Set the alarm and shut down the whole system. This 
  RTC_set_alarm(sleeptime); // parameters
  shutdown();
}

void read_and_send_data() {
  double temperature = 0, humidity = 0, bat_v = 0, ext_v = 0;

  // get sensor data
  if(init_sensor()) {
    temperature = get_temperature();
    humidity = get_humidity();
  } else {
    debug_println("AHT10 not found. Check your board.");
  }

  // measure voltages
  bat_v = read_voltage(h32_config.bat_v.pin, h32_config.bat_v.coefficient, h32_config.bat_v.constant);
  ext_v = read_voltage(h32_config.ext_v.pin, h32_config.ext_v.coefficient, h32_config.ext_v.constant);

  // send data if needed
  if(h32_config.api.type != 0) {
    api_calls[h32_config.api.type - 1](h32_config.api.key, h32_config.api.additional, temperature, humidity, bat_v, ext_v);      
  }

  // MQTT if needed
  if(strlen(h32_config.mqtt.server) != 0 && strlen(h32_config.mqtt.topic) != 0) {
    mqtt_call(temperature, humidity, bat_v, ext_v);
  }
}

/*
 * In the following functions we examine the button(s) and decide whether they
 * have been pressed long enough
 */
volatile uint32_t button_pressed = 0;
const int button_press_length = 1000;

/*
 * The button_interrupt_function() is set as the interrupt function, executed when the
 * button is pressed. To debounce only the first press is recorded.
 */
void button_interrupt_function()
{
  if(button_pressed == 0) {
    button_pressed = millis();  
  }
}
/*
 * This function simply tests if the button is pressed after "button_press_length"
 * has passed. This function is extremely simple and does not check whether 
 * the/a button has been released in the meantime.
 */
bool button_is_pressed() {
  if(button_pressed != 0) {
    if( (millis() - button_pressed) > button_press_length) {
      if(digitalRead(button) == LOW) {
        return true;
      }
      if(h32_config.trigger_pin > 0) {
        if(digitalRead(h32_config.trigger_pin) == LOW) {
          return true;
        }        
      }
    }
  }
  return false;
}

/*
 * Here starts the run part that is only executed if a button has been pressed
 */
bool portal_started = false;
uint8_t dot_count = 0;

/*
 * The loop function will be executed until either OTA has been executed, the 
 * Restart-button in the web interface has been pressed or the ESP has been reset.
 */
void loop() {
  // If we arrive here the button has been pressed

  // blink the LED in regular intervals to give a visual cue
  led_toggle();

  // Reset failed connections counter
  RTC_set_RAM(0);

  // this part simply adds a newline every 20 dots
  if (++dot_count >= 20) {
    debug_println(".");
    dot_count = 0;
  } else {
    debug_print(".");
  }
  delay(500);

  if (!portal_started) {
    ArduinoOTA.setHostname(h32_config.name);
    ArduinoOTA.begin();
    // Restart the Portal
    portal_started = start_Portal();
  }
  else {
    // Process actions from the portal
    wm.process();
    ArduinoOTA.handle();
  }
}

/*
 * The shutdown function pulls the DONE pin high until the power switch
 * turns off (a few seconds). This will cause a brownout which is of no
 * consequence. Thus we turn off the brownout detection and loop until
 * that happens.
 */
void shutdown() {
  debug_println("Shutdown");
  pinMode(DONE, OUTPUT);
  digitalWrite(DONE, HIGH);

  // turn off brownout detection and wait for power loss
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
  while(1) {;}
}
