// DONE LED ACTIVATION

/*
 * Since there is no built-in function to determine the current pinmode (needed for toggling),
 * we define our own function for that. This has been taken from:
 * https://github.com/arduino/Arduino/issues/4606
 * and modified to fit the ESP32 port and registér format
 */
int pinMode(uint8_t pin)
{
  if (pin >= NUM_DIGITAL_PINS) return (-1);

  uint32_t bit = digitalPinToBitMask(pin);
  uint32_t port = digitalPinToPort(pin);
  volatile uint32_t *reg = portModeRegister(port);
  if (*reg & bit) return (OUTPUT);

  volatile uint32_t *out = portOutputRegister(port);
  return ((*out & bit) ? INPUT_PULLUP : INPUT);
}

/*
 * The following functions allow to set a pin active or inactive. A pin number 0 is ignored.
 * An active state is a state where
 *     the pin is HIGH if the pin number is positive
 *     the pin is LOW if the pin number is negative
 * An inactive state is in tristate (there might still be a weak pull-up or pull-down resistor,
 * see datasheet ESP32 v3.8, table IO_MUX, p60)
 */
void pin_on(int8_t pin) {
  if(pin == 0) {
    return;
  }
  pinMode(pin, OUTPUT);
  if(pin > 0){
    // active high
    digitalWrite(pin, HIGH);
  } else {
    // active low
    digitalWrite(-pin, LOW);
  }
}
void pin_off(int8_t pin) {
  if(pin == 0) {
    return;
  }
  pinMode(pin, INPUT);
}
void pin_toggle(int8_t pin) {
  if(pin == 0) {
    return;
  }
  if(pinMode(pin) == OUTPUT) {
    pin_off(pin);
  } else {
    pin_on(pin);
  }
}
/*
 * The following functions abstract the LED control
 */

inline void led_toggle() {
  pin_toggle(h32_config.led_pin);
}

inline void led_on() {
  pin_on(h32_config.led_pin);
}

inline void led_off() {
  pin_off(h32_config.led_pin);
}
