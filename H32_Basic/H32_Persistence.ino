#include <FS.h>
#include <LittleFS.h>

#define SERIALIZE_2(doc, name) doc[#name] = (h32_config.name);
#define SERIALIZE_3(doc, part, name) doc[#part][#name] = (h32_config.part.name);

#define DESERIALIZE_2(doc, name) if(!(doc[#name]).isNull()){ (h32_config.name) = doc[#name]; };
#define DESERIALIZE_3(doc, part, name) if(!(doc[#part][#name]).isNull()){ (h32_config.part.name) = doc[#part][#name]; };

#define DESERIALIZE_SSID_2(doc, name) if(!(doc[#name]).isNull()){ strncpy(h32_config.name, doc[""#name], SSID_LENGTH); };
#define DESERIALIZE_TOPIC_3(doc, part, name) if(!(doc[#part][#name]).isNull()){ strncpy(h32_config.part.name, doc[#part][#name], TOPIC_LENGTH); };
#define DESERIALIZE_NAME_3(doc, part, name) if(!(doc[#part][#name]).isNull()){ strncpy(h32_config.part.name, doc[#part][#name], NAME_LENGTH); };


#include <Preferences.h>
Preferences prefs;

/*
 * Read the config object from persistent storage aka NVS using Preferences
 */
bool read_config() {
  if (!LittleFS.begin(false)) {
    debug_println("Couldn't mount LittleFS. Trying to format.");
    if (!LittleFS.begin(true)) {
      debug_println("ERROR: Couldn't format LittleFS. Aborting access.");
      return false;
    } else {
      debug_println("Formating LittleFS successful.");
    }
  }
  if (LittleFS.mkdir(h32_prefs_dir)) {
    debug_println("H32 config directory created or found");
  } else {
    debug_println("Coulnd't create H32 config directory");
  }
  File config_file = LittleFS.open(h32_prefs_path, "r");
  if (!config_file || config_file.isDirectory()) {
    debug_println("Cannot open config file for reading");
    write_config();
    return false;
  }
  StaticJsonDocument<json_doc_size> doc;
  auto error = deserializeJson(doc, config_file);
  if (error) {
    debug_println("Failed to parse config file");
    return false;
  }

  DESERIALIZE_2(doc, version);
  DESERIALIZE_2(doc, timeout);
  DESERIALIZE_SSID_2(doc, name);
  DESERIALIZE_2(doc, led_pin);
  DESERIALIZE_2(doc, trigger_pin);
  DESERIALIZE_3(doc, rtc, sleeptime);
  DESERIALIZE_3(doc, rtc, factor);
  DESERIALIZE_3(doc, rtc, limit);
  DESERIALIZE_3(doc, api, type);
  DESERIALIZE_NAME_3(doc, api, key);
  DESERIALIZE_NAME_3(doc, api, additional);
  DESERIALIZE_3(doc, bat_v, coefficient);
  DESERIALIZE_3(doc, bat_v, constant);
  DESERIALIZE_3(doc, bat_v, pin);
  DESERIALIZE_3(doc, bat_v, activation);
  DESERIALIZE_3(doc, ext_v, coefficient);
  DESERIALIZE_3(doc, ext_v, constant);
  DESERIALIZE_3(doc, ext_v, pin);
  DESERIALIZE_NAME_3(doc, mqtt, server);
  DESERIALIZE_3(doc, mqtt, port);
  DESERIALIZE_TOPIC_3(doc, mqtt, topic);
  DESERIALIZE_NAME_3(doc, mqtt, user);
  DESERIALIZE_NAME_3(doc, mqtt, passwd);
  DESERIALIZE_NAME_3(doc, ntp, server);
  DESERIALIZE_3(doc, ntp, daylightOffset_h);
  DESERIALIZE_3(doc, ntp, gmtOffset_h);

  config_file.close();
  LittleFS.end();

#ifdef H32_DEBUG
  serializeJson(doc, Serial);
  debug_println();
#endif // H32_DEBUG

  return true;
}

/*
 * Write the current configuration to NVS using Preferences
 */
bool write_config() {
  if (!LittleFS.begin(false)) {
    debug_println("Couldn't mount LittleFS. Trying to format.");
    if (!LittleFS.begin(true)) {
      debug_println("ERROR: Couldn't format LittleFS. Aborting access.");
      return false;
    } else {
      debug_println("Formating LittleFS successful.");
    }
  }
  if (LittleFS.mkdir(h32_prefs_dir)) {
    debug_println("H32 config directory created or found");
  } else {
    debug_println("Coulnd't create H32 config directory");
  }

  File config_file = LittleFS.open(h32_prefs_path, "w");
  if (!config_file) {
    debug_println("Cannot open config file for writing");
    return false;
  }
  StaticJsonDocument<json_doc_size> doc;

  SERIALIZE_2(doc, version);
  SERIALIZE_2(doc, timeout);
  SERIALIZE_2(doc, name);
  SERIALIZE_2(doc, led_pin);
  SERIALIZE_2(doc, trigger_pin);
  SERIALIZE_3(doc, rtc, sleeptime);
  SERIALIZE_3(doc, rtc, factor);
  SERIALIZE_3(doc, rtc, limit);
  SERIALIZE_3(doc, api, type);
  SERIALIZE_3(doc, api, key);
  SERIALIZE_3(doc, api, additional);
  SERIALIZE_3(doc, bat_v, coefficient);
  SERIALIZE_3(doc, bat_v, constant);
  SERIALIZE_3(doc, bat_v, pin);
  SERIALIZE_3(doc, bat_v, activation);
  SERIALIZE_3(doc, ext_v, coefficient);
  SERIALIZE_3(doc, ext_v, constant);
  SERIALIZE_3(doc, ext_v, pin);
  SERIALIZE_3(doc, mqtt, server);
  SERIALIZE_3(doc, mqtt, port);
  SERIALIZE_3(doc, mqtt, topic);
  SERIALIZE_3(doc, mqtt, user);
  SERIALIZE_3(doc, mqtt, passwd);
  SERIALIZE_3(doc, ntp, server);
  SERIALIZE_3(doc, ntp, daylightOffset_h);
  SERIALIZE_3(doc, ntp, gmtOffset_h);

  serializeJson(doc, config_file);
#ifdef H32_DEBUG
  serializeJson(doc, Serial);
  debug_println();
#endif // H32_DEBUG

  config_file.close();
  LittleFS.end();
  return true;
}
