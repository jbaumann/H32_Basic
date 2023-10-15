#include <Wire.h>

/*
 * These are the buttons in the root menu of the web portal
 */
std::vector<const char *> menu = {"wifi","param","info","sep","restart","update", "sep", "erase"};


/*
 * Initialize the WiFiManager instance
 */
int init_WiFiManager() {
  esp_wifi_set_country(&WM_COUNTRY_CN);  // Make sure all channels are selectable

  WiFi.mode(WIFI_STA); // explicitly set mode, esp defaults to STA+AP

  // set the hostname for the DNS
  wm.setHostname(h32_config.name);
  // Set the title for the configuration page
  wm.setTitle(h32_config.name);

  if (strlen(h32_config.static_conf.ip_address) != 0) {
    //set static ip
    IPAddress _ip, _gw, _sn;
    _ip.fromString(h32_config.static_conf.ip_address);
    if (strlen(h32_config.static_conf.gateway)) {
      _gw.fromString(h32_config.static_conf.gateway);
    }
    if (strlen(h32_config.static_conf.subnet)) {
      _sn.fromString(h32_config.static_conf.subnet);
    }
    wm.setSTAStaticIPConfig(_ip, _gw, _sn);
  }


  // Add the additional web pages
  wm.setWebServerCallback(bind_additional_web_pages);

  // Add the callback to save parameters
  wm.setSaveParamsCallback(saveParamsCallback);

  // Set the custom menu
  wm.setMenu(menu);
  
  // Add Parameters
  add_parameters();

  // if we already have WiFi data we won't start the portal
  // even if we cannot connect
  if(wm.getWiFiIsSaved()) {
    debug_println("Found a saved AP");
    wm.setEnableConfigPortal(false);
    wm.setConnectTimeout(h32_config.timeout);
  }

  // Now we try to connect
  bool res;
  res = wm.autoConnect(h32_config.name, ap_passwd); // password protected ap
  if(!res) {
      debug_print("Failed to connect to ");
      debug_println(wm.getWiFiSSID(true));
  } 
  else {
      debug_print("H32 is connected to ");
      debug_println(wm.getWiFiSSID(false));
  }
  return res;
}


/*
 * Create the name of the device, the string "H32-" followed by the device ID
 */
void create_AP_Name(char *ssid) {
  snprintf(ssid, SSID_LENGTH, "H32-%06X", (uint32_t)ESP.getEfuseMac());
}


/*
 * Start either the Captive Portal or the normal portal depending on WiFi connection status
 */
bool start_Portal() {
    bool res;
    wm.setEnableConfigPortal(true);
    wm.setConnectTimeout(0);

    if(WiFi.isConnected()){
      wm.startWebPortal();
      res = true;
    } else {
      res = wm.startConfigPortal(h32_config.name, ap_passwd);
    }
    return res;
}


/*
 * Bind the functions for the additional web pages
 */
void bind_additional_web_pages() {
  wm.server->on("/i2c_scan", handle_i2c_scan);
  wm.server->on("/devices", handle_devices);
  wm.server->on("/set_rtc", set_rtc);
#ifdef H32_DEBUG
  wm.server->on("/set_rtc_debug", set_rtc_debug);
#endif // H32_DEBUG
}
