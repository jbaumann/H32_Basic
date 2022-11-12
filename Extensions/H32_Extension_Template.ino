/*
 * This class allows to implement user-specific code for reading additional sensors.
 * Different methods can be implemented to allow execution of your code at a
 * specific time.
 */


/*
 * Rename the following class UserExtension and its constructor to something
 * that describes your sensor or device.
 * If you need your own data, add it as private variables. We will create an
 * instance of this class so you can safely assume that these attributes will
 * be available.
 */
class UserExtension : public Extension {
private:
protected:
public:
  // Optional, if "send to back" is needed (e.g. for a display)
  // UserExtension(bool sendToBack = false) : Extension(sendToBack){ ; };

  /*
   * The init() method is used to initializes your sensor and instance data.
   * It receives the current measurements as a parameter should it need them.
   * @return true if successful
   */
  bool init(H32_Measurements &measurements);
  /*
   * The init() method is used to initializes your sensor and instance data.
   * It receives the current measurements as a parameter should it need them.
   * @return true if successful
   */
  bool wiFiInitialized(bool wiFiInitialized);
  /*
   * The read() method is used to read the sensor and store its data locally.
   * Access to the basic H32 measurements is provided via the parameter if
   * needed.
   * @return true if successful
   */
  bool read(H32_Measurements &measurements);
  /*
   * The collect() method is used to collect your sensor data into the map
   * that is provides as a reference parameter. Data you add here will be
   * added to the JSON that is sent via MQTT and to IOTPLotter.
   * @return true if successful
   */
  bool collect(unordered_map<char *, double> &data);
  /*
   * the api_call() method allows to implement your own method of sending data.
   * It gets the same data as parameters as the original api_call methods.
   * If you use this method, then you should set the api call type to "unset"
   * and put the data in the API settings
   * @return true if successful
   */
   bool api_call(char* api_key, char *api_additional, 
        H32_Measurements &measurements, unordered_map<char *, double> &additional_data);

};
namespace {
  /*  
   * This creation of an object of your extension code is necessary so 
   * that you can keep your own data as instance attributes and furthermore,
   * to register the code with the extension mechanism. Creating the object
   * is enough to let the system execute your code at the respective times.
   * To send the extension to the back of the queue, use the parameter like
   * this:
   * Extension *extension = new UserExtension(true);
   */
  Extension *extension = new UserExtension();
}

bool UserExtension::init(H32_Measurements &measurements) { 
  debug_println("UserExtension Init");
  return true;
 };
bool UserExtension::wiFiInitialized(bool wiFiInitialized) { 
  debug_println("UserExtension WiFi Init");
  return true;
 };
bool UserExtension::read(H32_Measurements &measurements) {
  debug_println("UserExtension Read");
  return true;
};
bool UserExtension::collect(unordered_map<char *, double> &data) {
  debug_println("UserExtension Collect");
  return true;
};
bool UserExtension::api_call(char* api_key, char *api_additional, 
        H32_Measurements &measurements, unordered_map<char *, double> &additional_data) {
  debug_println("UserExtension: Default API Call");
  return true;
};
