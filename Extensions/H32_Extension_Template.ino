/*
 * This class allows to implement user-specific code for reading additional sensors.
 * Different methods can be implemented to allow execution of your code at a
 * specific time.
 */

#include <vector>

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
   */
  void init(H32_Measurements &measurements);
  /*
   * The read() method is used to read the sensor and store its data locally.
   * Access to the basic H32 measurements is provided via the parameter if
   * needed.
   */
  void read(H32_Measurements &measurements);
  /*
   * The collect() method is used to collect your sensor data into the map
   * that is provides as a reference parameter. Data you add here will be
   * added to the JSON that is sent via MQTT and to IOTPLotter.
   */
  void collect(unordered_map<char *, double> &data);
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

void UserExtension::init(H32_Measurements &measurements) { 
  debug_println("UserExtension Init");
 };
void UserExtension::read(H32_Measurements &measurements) {
  debug_println("UserExtension Read");
};
void UserExtension::collect(unordered_map<char *, double> &data) {
  debug_println("UserExtension Send");
};
