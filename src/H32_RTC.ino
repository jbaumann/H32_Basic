/*
 * Here we have all functions that communicate with the RTC.
 * We are using the real time clock, the alarm and the one
 * byte of RAM.
 */

/* get a real time clock object */
PCF85063A rtc;

/*
 * Setting the time is simply a call to the rtc object
 */
void RTC_set_time(tm *time_info) {
  debug_print("Setting the RTC to: ");
  debug_println(time_info, "%H:%M:%S, %B %d %Y");
  rtc.time_set(time_info);
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
 */
PCF85063A_Regs RTC_stop_and_check() {
  PCF85063A_Regs regs = 0;
  PCF85063A_Regs result = 0;

  // get the registers
  rtc.ctrl_get(&regs);
  // stop any countdown
  rtc.countdown_set(false, PCF85063A::CNTDOWN_CLOCK_1HZ, 0, false, false);

  // clean alarm and countdown bits
  PCF85063A_Regs new_regs = regs;
  PCF85063A_REG_SET(new_regs, PCF85063A_REG_AF);
  PCF85063A_REG_SET(new_regs, PCF85063A_REG_TF);

  // Check for alarms
  if (PCF85063A_REG_GET(regs, PCF85063A_REG_AF))
  {
    result |= PCF85063A_REG_AF;
    PCF85063A_REG_CLEAR(new_regs, PCF85063A_REG_AF);
  }
  // Check for countdown
  if (PCF85063A_REG_GET(regs, PCF85063A_REG_TF))
  {
    result |= PCF85063A_REG_TF;
    PCF85063A_REG_CLEAR(new_regs, PCF85063A_REG_TF);
  }
  // PCF85063A_REG_CLEAR(new_regs, result);

  rtc.ctrl_set(new_regs, false);
  
  return result;
}

/*
 * This function takes a sleeptime in seconds, calculates a new
 * alarm time from that and the current time of the RTC, and
 * then sets it.
 */
bool RTC_set_alarm(int32_t sleeptime) {
  debug_println("Set Alarm");
  tm time_info;
  tm time_added;

  // get current time from RTC
  rtc.time_get(&time_info);
  debug_print("Current Time: ");
  debug_println(&time_info, "%H:%M:%S, %B %d %Y");

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

  return rtc.alarm_set(&time_info, true);
}

/*
 * Get the byte stored in the RTC RAM
 */
int16_t RTC_get_RAM() {
  return rtc.ram_get();
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
