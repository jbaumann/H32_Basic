/*
 * The following functions abstract the LED control
 */
void led_init() {
  if(h32_config.led_pin == -1) {
    return;
  }
  pinMode(h32_config.led_pin, OUTPUT);
}

void led_toggle() {
  if(h32_config.led_pin == -1) {
    return;
  }
  digitalWrite(h32_config.led_pin, !digitalRead(h32_config.led_pin));
}

void led_on() {
  if(h32_config.led_pin == -1) {
    return;
  }
  digitalWrite(h32_config.led_pin, HIGH);
}

void led_off() {
  if(h32_config.led_pin == -1) {
    return;
  }
  digitalWrite(h32_config.led_pin, LOW);
}
