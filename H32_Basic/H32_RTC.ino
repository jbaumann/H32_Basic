/*
 * Here we have all functions that communicate with the RTC.
 * We are using the real time clock, the alarm and the one
 * byte of RAM.
 */


/* get a real time clock object */
PCF85063A rtc;

/*
 * We are setting the time as part of the portal mode, when the
 * user presses the button to set the time to the NTP time.
 * To ensure that the RTC is in a well-defined state (e.g., after
 * exchanging the battery) we reset the RTC registers first.
 */
void RTC_set_time(tm *time_info) {
  rtc.reset();
  RTC_stop_and_check();
  rtc.time_set(time_info);
  debug_print("Setting the RTC to: ");
  debug_println(time_info, "%H:%M:%S, %B %d %Y");  
}

/*
 * Getting the time is simply a call to the rtc object
 */
void RTC_get_time(tm *time_info) {

  rtc.time_get(time_info);

  debug_print("Returning RTC values: ");
  debug_println(time_info, "%H:%M:%S, %B %d %Y");
}
/*
 * The following function stops any countdown or alarm
 * after checking it. The result is returned.
 * Sooner or later this should be refactored into the class.
 */
PCF85063A_Regs RTC_stop_and_check() {
  PCF85063A_Regs regs = 0;
  //PCF85063A_Regs result = 0;

  // Get the registers
  rtc.ctrl_get(&regs);

  // stop any countdown
  rtc.countdown_set(false, PCF85063A::CNTDOWN_CLOCK_1HZ, 0, false, false);

  // Clear alarm and countdown bits
  PCF85063A_Regs new_regs = regs;
  PCF85063A_REG_SET(new_regs, PCF85063A_REG_AF);
  PCF85063A_REG_SET(new_regs, PCF85063A_REG_TF);

  // Check for alarms
  if (PCF85063A_REG_GET(regs, PCF85063A_REG_AF))
  {
    //result |= PCF85063A_REG_AF;
    PCF85063A_REG_CLEAR(new_regs, PCF85063A_REG_AF);
  }
  // Check for countdown
  if (PCF85063A_REG_GET(regs, PCF85063A_REG_TF))
  {
    //result |= PCF85063A_REG_TF;
    PCF85063A_REG_CLEAR(new_regs, PCF85063A_REG_TF);
  }
  // Clear the alarm enable bit
  PCF85063A_REG_CLEAR(new_regs, PCF85063A_REG_AIE);

  rtc.ctrl_set(new_regs, false);

  //result |= regs & 0xFF;
  //return result;
  return regs;
}

/*
 * This function takes a sleeptime in seconds, calculates a new
 * alarm time from that and the current time of the RTC, and
 * then sets it.
 */
bool RTC_set_alarm(int32_t sleeptime) {
  debug_println("Set Alarm");
  RTC_stop_and_check();
 
  tm time_info;
  tm time_added;

  // get current time from RTC
  bool osc_runs = rtc.time_get(&time_info);

  mktime(&time_info);

  // This code is for when I work out how to force the osc to start
  // I think this is done when the time gets set
  if (!osc_runs) {
    RTC_set_time(&time_info);
    // Set the RTC clock time here if it's not running
    debug_print("osc not running, so started");
    //rtc.start(); // Replace with the appropriate method to start the RTC oscillator
  }

  // create tm from sleeptime
  time_added.tm_sec = sleeptime % 60;
  sleeptime /= 60;
  time_added.tm_min = sleeptime % 60;
  sleeptime /= 60;
  time_added.tm_hour = sleeptime % 24;
  sleeptime /= 24;
  // with these divisions the result is sure to fit into an int
  time_added.tm_mday = sleeptime;

  // add to time_info
  time_info.tm_sec  += time_added.tm_sec;
  time_info.tm_min  += time_added.tm_min;
  time_info.tm_hour += time_added.tm_hour;
  time_info.tm_mday += time_added.tm_mday;
  //time_info.tm_mon  += time_added.tm_mon;
  //time_info.tm_year += time_added.tm_year;

  // normalize
  mktime(&time_info);
 
  // set rtc alarm
  debug_print("Wakeup Time: ");
  debug_println(&time_info, "%H:%M:%S, %B %d %Y");
 
  tm timenow;
  RTC_get_time(&timenow);
  return rtc.alarm_set(&time_info, true);
}// end of RTC_set_alarm

/*
 * Get the byte stored in the RTC RAM
 */
int16_t RTC_get_RAM() {
  int16_t result = rtc.ram_get();
  if(result == -1) {
    // If reading the RTC value is unsuccessful we return 0
    result = 0;
  }
  return result;
}

/*
 * Set the byte stored in the RTC RAM
 */
bool RTC_set_RAM(uint8_t ram) {
  return rtc.ram_set(ram);
}

/*
 * Increment the byte stored in the RTC RAM.
 * If 255 keep it there.
 */
bool RTC_increment_RAM() {
  int16_t value = rtc.ram_get();
  if(value == -1) {
    return false;
  }
  if(value < 255) {
    value +=1;
    return rtc.ram_set(value);
  } else {
    return true;
  }
}

