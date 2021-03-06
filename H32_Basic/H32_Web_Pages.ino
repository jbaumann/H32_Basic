/*
   Here we create the additional pages that show the results of the i2c scan,
   the sensor data and the page that you never see that sets the RTC.
*/

/*
   Create a simple HTML page that lists all devices on the local I2C bus
*/
void handle_i2c_scan() {
  debug_println("[HTTP] handle i2cscan");

  debug_println("I2C scanner. Scanning ...");
  byte count = 0;

  String output = "<head>";
  output += FPSTR(HTTP_STYLE);
  output += "</head><h1>I2C Scan</h1><hr>";
  String footer = "<hr/><a href='/param' class='D'>Back</a></body>";

  Wire.begin();
  for (byte i = 8; i < 120; i++)
  {
    Wire.beginTransmission (i);          // Begin I2C transmission Address (i)
    if (Wire.endTransmission () == 0)  // Receive 0 = success (ACK response)
    {
      debug_print("Found address: ");
      debug_print(i, DEC);
      debug_print(" (0x");
      debug_print(i, HEX);
      debug_println(")");
      output += "Found address: 0x" + String(i, HEX) + "<br>";
      count++;
    }
  }
  debug_print ("Found ");
  debug_print (count, DEC);        // numbers of devices
  debug_println (" device(s).");
  output += "<p>Found " + String(count, DEC) + " device(s).</p>";
  output += footer;

  wm.server->send(200, "text/html", output);
}


/*
   Create a simple HTML page that shows Sensor and voltage readings, RTC and NTP time and allows to set the RTC
*/
void handle_devices() {
  debug_println("[HTTP] handle devices");

  debug_println("Device page. Reading data, getting NTP and RTC time...");
  byte count = 0;

  String output = "<head>";
  output += FPSTR(HTTP_STYLE);
  output += "</head><body><h1>Devices</h1><h2>RTC Time</h2>";
  String footer = "<a href='/param' class='D'>Back</a></body>";

  /*
     First the NTP time
  */
  tm timeinfo;
  char buf[64];
  const char *format = "%H:%M:%S, %B %d %Y";
  size_t written;

  configTime(h32_config.ntp.gmtOffset_h * 3600, h32_config.ntp.daylightOffset_h * 3600, h32_config.ntp.server);
  if (!getLocalTime(&timeinfo)) {
    output += "<p>NTP Time: Faild to obtain NTP time.</p>";
  } else {
    written = strftime(buf, 64, format, &timeinfo);
    debug_println(buf);
    output += "<p>NTP Time: " + String(buf) + ".</p>";
  }
  // RTC time
  RTC_get_time(&timeinfo);
  written = strftime(buf, 64, format, &timeinfo);
  debug_println(buf);
  output += "<p>RTC Time: " + String(buf) + ".</p>";
  output += "<form action='/set_rtc'  method='get'><button>Set RTC Time</button></form><hr/>";

  H32_Measurements m;
  m.readMeasurements();

  output += "<h2>AHT Sensor</h2>";
  if (m.isInitSuccessful()) {
    output += "<p>Temperature: " + String(m.getTemperature()) + " degrees C</p>";
    output += "<p>Humidity: " + String(m.getHumidity()) + "% rH</p><hr/>";
  } else {
    output += "AHT10 not found. Check your board.<hr/>";
  }

  output += "<h2>Voltages</h2>";
  output += "<p>Battery Voltage: ";
  output += String(m.getBatV()) + "V</p>";
  output += "<p>Ext Voltage: ";
  output += String(m.getExtV()) + "V</p><hr/>";
  output += footer;
  wm.server->send(200, "text/html", output);
}


/*
   Set the RTC with the current NTP time and forward the browser back to the devices page
*/
void set_rtc() {
  tm timeinfo;

  configTime(h32_config.ntp.gmtOffset_h * 3600, h32_config.ntp.daylightOffset_h * 3600, h32_config.ntp.server);
  if (getLocalTime(&timeinfo)) {
    RTC_set_time(&timeinfo);
  }

  // Redirect the browser back to "/devices"
  wm.server->sendHeader("Location", "/devices", true);
  wm.server->send(307, "text/plain");
}
