# H32_Basic - A Firmware for the H32 board that allows to directly start using the board productively.

This is a firmware for the H32 board by Burgduino providing the following functionality (without any particular order):
* OTA Updates
* Portal for entering credentials
* Portal allows to enter additional configuration data
* GPIO0 leads to config portal after start (i.e. after LED is turned on)
* A second GPIO pin is configurable as additional trigger pin
* Store Data in NVS
* Configurable LED pin
* Page that allows scanning for I2C devices
* Page showing the current measurements (sensor and voltages)
* Thingspeak communication
* IOTPlotter Communication
* MQTT
* Portal allows to set the RTC to NTP time
* Failed Connection Counter stored in RTC memory
* Dynamic, configurable increase of sleep time when WiFi is not reachable

The following third-party libraries are used in this sketch:
*   WiFiManager by tzapu
*   Adafruit_AHTX0 by Adafruit
*   Thingspeak by Mathworks
*   Arduino Client for MQTT by Nick O’Leary

These can be installed using the library manager of the Arduino IDE (or downloaded from Github). An additional library for the PCF85063 by Jaakko Salo has been modified to quite some extent and is directly included.
