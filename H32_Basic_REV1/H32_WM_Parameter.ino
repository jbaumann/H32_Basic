/*
 * Following is everything having to do with creating, reading and writing the parameters
 * that are shown on the "/param" page in the web interface.
 */

/*
 * Create Parameters reflecting the config struct
 */
// Tools -------------------
String version_str = String(H32_MAJOR) + "." + String(H32_MINOR) + "." + String(H32_PATCH);  
String header_text = String("<h1>Config page for H32</h1><p>firmware version: ") + version_str + "</p>";
WiFiManagerParameter header(header_text.c_str());
WiFiManagerParameter i2c_scan_link("<h1>Tools</h1><p><a href='/i2c_scan' class='D'>Scan I2C Bus</a></p>");
WiFiManagerParameter rtc_link("<p><a href='/devices' class='D'>Show Device Readings (and set RTC)</a></p><hr/><H1>Settings</H1>");
// Basic Settings ----------
WiFiManagerParameter basic_heading("<h2>Basic</h2>");
WiFiManagerParameter basic_name("basic_name", "Device Name", "", SSID_LENGTH);
WiFiManagerParameter basic_led_pin("basic_led_pin", "LED Pin (-1 turns off)", "", U8_LENGTH, "pattern='-?\\d{0,2}'");
WiFiManagerParameter basic_trigger_pin("basic_trigger_pin", "Additional Trigger Pin (-1 or 0 turns off)", "", U8_LENGTH, "pattern='-?\\d{0,2}'");
WiFiManagerParameter basic_timeout("basic_timeout", "WiFi Connection Timeout", "", U16_LENGTH);
// RTC Settings
WiFiManagerParameter rtc_heading("<h2>RTC</h2>");
WiFiManagerParameter rtc_sleeptime("rtc_sleeptime", "RTC Sleep Time in seconds", "", U32_LENGTH);
WiFiManagerParameter rtc_factor("rtc_factor", "RTC Backoff Factor", "", DOUBLE_LENGTH);
WiFiManagerParameter rtc_limit("rtc_limit", "RTC Backoff Limit", "", DOUBLE_LENGTH);
// Measurements ------------
WiFiManagerParameter measurement_heading("<h2>Measurements</h2>");
WiFiManagerParameter measurement_bat_coefficient("measurement_bat_coefficient", "Battery Voltage Compensation Coefficient", "", DOUBLE_LENGTH);
WiFiManagerParameter measurement_bat_constant("measurement_bat_constant", "Battery Voltage Compensation Constant", "", DOUBLE_LENGTH);
WiFiManagerParameter measurement_bat_pin("measurement_bat_pin", "Battery Voltage Pin", "", U8_LENGTH, "pattern='\\d{0,2}'");
WiFiManagerParameter measurement_ext_coefficient("measurement_ext_coefficient", "Ext Voltage Compensation Coefficient", "", DOUBLE_LENGTH);
WiFiManagerParameter measurement_ext_constant("measurement_ext_constant", "Ext Voltage Compensation Constant", "", DOUBLE_LENGTH, "pattern='[+-]?[.\\d]{0,6}'");
WiFiManagerParameter measurement_ext_pin("measurement_ext_pin", "Ext Voltage Pin", "", U8_LENGTH, "pattern='\\d{0,2}'");
// API Keys ----------------
WiFiManagerParameter api_heading("<h2>Service API Keys</h2>");
WiFiManagerParameter *api_type; // will be created dynamically
String api_type_str;
WiFiManagerParameter api_type_display("api_type_display", "<br/> <br/>", "", NAME_LENGTH, "readonly", WFM_LABEL_AFTER);
WiFiManagerParameter api_key("api_key", "API Key", "", NAME_LENGTH);
WiFiManagerParameter api_additional("api_additional", "API Additional Value", "", NAME_LENGTH);
// MQTT Settings -----------
WiFiManagerParameter mqtt_heading("<h2>MQTT</h2>");
WiFiManagerParameter mqtt_server("mqtt_server", "MQTT Server", "", NAME_LENGTH);
WiFiManagerParameter mqtt_port("mqtt_port", "MQTT Port", "", U16_LENGTH, "pattern='\\d{0,5}'");
WiFiManagerParameter mqtt_topic("mqtt_topic", "MQTT Topic", "", TOPIC_LENGTH);
WiFiManagerParameter mqtt_user("mqtt_user", "MQTT User", "", NAME_LENGTH);
WiFiManagerParameter mqtt_password("mqtt_password", "MQTT Password", "", NAME_LENGTH);
// NTP Settings -----------
WiFiManagerParameter ntp_heading("<h2>NTP</h2>");
WiFiManagerParameter ntp_server("ntp_server", "NTP Server", "", NAME_LENGTH);
WiFiManagerParameter ntp_daylight_offset("ntp_daylight_offset", "NTP Daylight Offset", "", U8_LENGTH, "pattern='\\d{0,2}'");
WiFiManagerParameter ntp_timezone_offset("ntp_timezone_offset", "NTP Timezone Offset", "", U8_LENGTH, "pattern='\\d{0,2}'");

/*
 * Add parameters to WifiManager and set their values to the ones in the config struct
 */
void add_parameters() {
  // Header
  wm.addParameter(&header);
  // Tools
  wm.addParameter(&i2c_scan_link);
  wm.addParameter(&rtc_link);

  // Basic Settings
  wm.addParameter(&basic_heading);
  wm.addParameter(&basic_name);
  basic_name.setValue(h32_config.name, SSID_LENGTH);
  wm.addParameter(&basic_led_pin);
  basic_led_pin.setValue(String(h32_config.led_pin).c_str(), U8_LENGTH);
  wm.addParameter(&basic_trigger_pin);
  basic_trigger_pin.setValue(String(h32_config.trigger_pin).c_str(), U8_LENGTH);
  wm.addParameter(&basic_timeout);
  basic_timeout.setValue(String(h32_config.timeout).c_str(), U16_LENGTH);

  // RTC Settings
  wm.addParameter(&rtc_heading);
  wm.addParameter(&rtc_sleeptime);
  rtc_sleeptime.setValue(String(h32_config.rtc.sleeptime).c_str(), U32_LENGTH);
  wm.addParameter(&rtc_factor);
  rtc_factor.setValue(String(h32_config.rtc.factor).c_str(), DOUBLE_LENGTH);
  wm.addParameter(&rtc_limit);
  rtc_limit.setValue(String(h32_config.rtc.limit).c_str(), DOUBLE_LENGTH);

  // Measurement Settings
  wm.addParameter(&measurement_heading);
  wm.addParameter(&measurement_bat_coefficient);
  measurement_bat_coefficient.setValue(String(h32_config.bat_v.coefficient).c_str(), DOUBLE_LENGTH);
  wm.addParameter(&measurement_bat_constant);
  measurement_bat_constant.setValue(String(h32_config.bat_v.constant).c_str(), DOUBLE_LENGTH);
  wm.addParameter(&measurement_bat_pin);
  measurement_bat_pin.setValue(String(h32_config.bat_v.pin).c_str(), U8_LENGTH);
  wm.addParameter(&measurement_ext_coefficient);
  measurement_ext_coefficient.setValue(String(h32_config.ext_v.coefficient).c_str(), DOUBLE_LENGTH);
  wm.addParameter(&measurement_ext_constant);
  measurement_ext_constant.setValue(String(h32_config.ext_v.constant).c_str(), DOUBLE_LENGTH);
  wm.addParameter(&measurement_ext_pin);
  measurement_ext_pin.setValue(String(h32_config.ext_v.pin).c_str(), U8_LENGTH);

  // API Settings
  wm.addParameter(&api_heading);

  String api_type_pre = "<label for='api_type_id'>Service Type (Choose from Dropdown, current value below)</label><br/><select id='api_type_id' name='api_type_id'>";
  api_type_str += api_type_pre + "<option value='-1' selected>Don't Change</option>";
  for(int i = 0; i < apitype_num; i++) {
    api_type_str += "<option value='" + String(i) + "'>" + apitype_names[i] + "</option>";
  }
  api_type_str += "</select>";
  api_type = new WiFiManagerParameter(api_type_str.c_str());
  wm.addParameter(api_type);
  wm.addParameter(&api_type_display);
  api_type_display.setValue((String("Current Value: ") + apitype_names[h32_config.api.type]).c_str(), NAME_LENGTH);
  
  wm.addParameter(&api_key);
  api_key.setValue(h32_config.api.key, NAME_LENGTH);

  wm.addParameter(&api_additional);
  api_additional.setValue(h32_config.api.additional, NAME_LENGTH);

  // MQTT
  wm.addParameter(&mqtt_heading);
  wm.addParameter(&mqtt_server);
  mqtt_server.setValue(h32_config.mqtt.server, NAME_LENGTH);
  wm.addParameter(&mqtt_port);
  mqtt_port.setValue(String(h32_config.mqtt.port).c_str(), U16_LENGTH);  
  wm.addParameter(&mqtt_topic);
  mqtt_topic.setValue(h32_config.mqtt.topic, TOPIC_LENGTH);
  wm.addParameter(&mqtt_user);
  mqtt_user.setValue(h32_config.mqtt.user, NAME_LENGTH);
  wm.addParameter(&mqtt_password);
  mqtt_password.setValue(h32_config.mqtt.passwd, NAME_LENGTH);

  // NTP
  wm.addParameter(&ntp_heading);
  wm.addParameter(&ntp_server);
  ntp_server.setValue(h32_config.ntp.server, NAME_LENGTH);
  wm.addParameter(&ntp_timezone_offset);
  ntp_timezone_offset.setValue(String(h32_config.ntp.gmtOffset_h).c_str(), U8_LENGTH);
  wm.addParameter(&ntp_daylight_offset);
  ntp_daylight_offset.setValue(String(h32_config.ntp.daylightOffset_h).c_str(), U8_LENGTH);
}

/*
 * Retrieve parameters, store them in the config and save that to the persistent storage
 */
void saveParamsCallback () {
  debug_println("Save Params Callback");

  // Basic Settings
  retrieve_from_form(h32_config.name, &basic_name, SSID_LENGTH);
  retrieve_from_form(&h32_config.led_pin, &basic_led_pin);
  retrieve_from_form(&h32_config.trigger_pin, &basic_trigger_pin);
  retrieve_from_form(&h32_config.timeout, &basic_timeout);

  // RTC Settings
  retrieve_from_form(&h32_config.rtc.sleeptime, &rtc_sleeptime);
  retrieve_from_form(&h32_config.rtc.factor, &rtc_factor);
  retrieve_from_form(&h32_config.rtc.limit, &rtc_limit);

  // Measurement Settings
  retrieve_from_form(&h32_config.bat_v.coefficient, &measurement_bat_coefficient);
  retrieve_from_form(&h32_config.bat_v.constant, &measurement_bat_constant);
  retrieve_from_form(&h32_config.bat_v.pin, &measurement_bat_pin);

  retrieve_from_form(&h32_config.ext_v.coefficient, &measurement_ext_coefficient);  
  retrieve_from_form(&h32_config.ext_v.constant, &measurement_ext_constant);
  retrieve_from_form(&h32_config.ext_v.pin, &measurement_ext_pin);

  // API Settings
  if(wm.server->hasArg("api_type_id")) {
    String value = wm.server->arg("api_type_id");
    APIType api_type_val = (APIType)atoi(value.c_str());
    if(api_type_val != -1) {
      h32_config.api.type = api_type_val;
      api_type_display.setValue(apitype_names[h32_config.api.type], NAME_LENGTH);
    }
    debug_print("API Type: ");
    debug_println(h32_config.api.type);
  }

  retrieve_from_form(h32_config.api.key, &api_key, NAME_LENGTH);
  retrieve_from_form(h32_config.api.additional, &api_additional, NAME_LENGTH);

  // MQTT
  retrieve_from_form(h32_config.mqtt.server, &mqtt_server, NAME_LENGTH);
  retrieve_from_form(&h32_config.mqtt.port, &mqtt_port);
  retrieve_from_form(h32_config.mqtt.topic, &mqtt_topic, TOPIC_LENGTH);
  retrieve_from_form(h32_config.mqtt.user, &mqtt_user, NAME_LENGTH);    
  retrieve_from_form(h32_config.mqtt.passwd, &mqtt_password, NAME_LENGTH);

  // NTP
  retrieve_from_form(h32_config.ntp.server, &ntp_server, NAME_LENGTH);
  retrieve_from_form(&h32_config.ntp.gmtOffset_h, &ntp_timezone_offset);
  retrieve_from_form(&h32_config.ntp.daylightOffset_h, &ntp_daylight_offset);

  write_config();
}

void retrieve_from_form(char *config_val, WiFiManagerParameter *wmp, int length) {  
  strncpy(config_val, wmp->getValue(), length);
  debug_print(wmp->getLabel());
  debug_print(" (c*): ");
  debug_println(config_val);
}
void retrieve_from_form(double *config_val, WiFiManagerParameter *wmp) {
  *config_val = atof(wmp->getValue());
  debug_print(wmp->getLabel());
  debug_print(" (d): ");
  debug_println(*config_val);
}
void retrieve_from_form(int8_t *config_val, WiFiManagerParameter *wmp) {
  *config_val = atoi(wmp->getValue());
  debug_print(wmp->getLabel());
  debug_print(" (i8): ");
  debug_println(*config_val);
}
void retrieve_from_form(uint16_t *config_val, WiFiManagerParameter *wmp) {
  *config_val = atoi(wmp->getValue());
  debug_print(wmp->getLabel());
  debug_print(" (i16): ");
  debug_println(*config_val);    
}
void retrieve_from_form(uint32_t *config_val, WiFiManagerParameter *wmp) {
  *config_val = atol(wmp->getValue());
  debug_print(wmp->getLabel());
  debug_print(" (i32): ");
  debug_println(*config_val);  
}
