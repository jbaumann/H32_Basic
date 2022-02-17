/*
 * This function reads a voltage at the given pin with an oversampling
 * of "oversampling" times and removes the highest and lowest value from
 * the result.
 * We the compensate for intrinsic measurement offsets and integral non-linearity
 * by using a coefficient (the factor) and a constant (added to the result) for a 
 * correction.
 */

const uint8_t oversampling = 5;

double read_voltage (uint8_t pin, double coefficient, double constant) {
  
  uint32_t adc_temp = 0;
  uint32_t adc_lowest = UINT32_MAX;
  uint32_t adc_highest = 0;

  // Read the voltage
  for(int i = 0; i < oversampling + 2; i++) {
    uint32_t new_val = analogReadMilliVolts(pin);
    adc_temp += new_val;
    if(new_val < adc_lowest ) {
      adc_lowest = new_val;
    }
    if(new_val > adc_highest) {
      adc_highest = new_val;
    }
  }
  adc_temp -= adc_lowest + adc_highest;
  debug_print("ADC Value for pin ");
  debug_print(pin);
  debug_print(": ");
  debug_println(adc_temp / oversampling);

  // adjust the result of the ADC
  adc_temp = (adc_temp * coefficient) / oversampling + constant * 1000;
  
  // Rounding the result to two decimal places
  double result = ((adc_temp + 5) / 10) / 100.0;
  debug_print("Voltage for pin ");
  debug_print(pin);
  debug_print(": ");
  debug_println(result);
  return result;
}

double read_bat_voltage() {
 if(h32_config.bat_v.activation != 0) {
    pin_on(h32_config.bat_v.activation);
  }
  double bat_v = read_voltage(h32_config.bat_v.pin, h32_config.bat_v.coefficient, h32_config.bat_v.constant);
  if(h32_config.bat_v.activation != 0) {
    pin_off(h32_config.bat_v.activation);
  }
  return bat_v;
}

double read_ext_voltage() {
  return read_voltage(h32_config.ext_v.pin, h32_config.ext_v.coefficient, h32_config.ext_v.constant);
}
