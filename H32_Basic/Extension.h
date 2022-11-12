#ifndef EXTENSION_H
#define EXTENSION_H

#include <vector>

#include "H32_Measurements.h"

using namespace std;

class Extension {
private:
  static vector<Extension *> *userFunctions;
protected:
  static void registerUserExtension(Extension *newExtension, bool sendToBack = false);
  Extension(bool sendToBack = false) { registerUserExtension(this, sendToBack); };

public:
  static inline bool hasEntries() { return userFunctions != NULL; };
  static inline vector<Extension *> * getContainer() { return userFunctions; };

  virtual bool init(H32_Measurements &measurements) { 
    debug_println("Extension: Default Init");
    return true;
  };
  virtual bool wiFiInitialized(bool wiFiInitialized) {
    debug_println("Extension: Default wiFiInitialized");
    return true;
  };
  virtual bool read(H32_Measurements measurements) {
    debug_println("Extension: Default Read");
    return true;
  };
  virtual bool collect(unordered_map<char *, double> &data) {
    debug_println("Extension: Default Collect");
    return true;
  };
  virtual bool api_call(char* api_key, char *api_additional, 
          H32_Measurements &measurements, unordered_map<char *, double> &additional_data) {
     debug_println("Extension: Default API Call");
     return true;
  };
};
// Out-of-line initialization for non-const static members
vector<Extension *>  *Extension::userFunctions = NULL;

void Extension::registerUserExtension(Extension *newExtension, bool sendToBack) {
  if (userFunctions == NULL) {
    userFunctions = new vector<Extension *>();
  }
  if (sendToBack) {
    userFunctions->push_back(newExtension);
  } else {
    userFunctions->insert(userFunctions->begin(), newExtension);
  }
}
#endif // EXTENSION_H
