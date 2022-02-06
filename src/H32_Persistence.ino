#include <Preferences.h>
Preferences prefs;

/*
 * Read the config object from persistent storage aka NVS using Preferences
 */
bool read_config() {
  prefs.begin(h32_prefs_key); // use "h32_config" namespace

  size_t len = prefs.getBytesLength(h32_prefs_key);

  debug_print("Read: H32_config size: ");
  debug_println(sizeof(H32_Config_t));
  debug_print("Read: Prefs len: ");
  debug_println(len);

  // Currently we do not use the version information stored in the config.
  // We simply check for the same size. If there is a difference, we write
  // a new initial configuration.
  if(len != sizeof(H32_Config_t)) {
    // incompatible version of the data or none at all
    return write_config();
  }

  prefs.getBytes(h32_prefs_key, (void *)&h32_config, len);

  prefs.end();

  return true;
}

/*
 * Write the current configuration to NVS using Preferences
 */
bool write_config() {
  prefs.begin(h32_prefs_key); // use "h32_config" namespace

  prefs.putBytes(h32_prefs_key, (void *)&h32_config, sizeof(H32_Config_t));

  size_t len = prefs.getBytesLength(h32_prefs_key);

  debug_print("Write: H32_config size: ");
  debug_println(sizeof(H32_Config_t));
  debug_print("Write: Prefs len: ");
  debug_println(len);

  prefs.end();
  
  if(len != sizeof(H32_Config_t)) {
    return false;
  } else {
    return true;
  }
}
