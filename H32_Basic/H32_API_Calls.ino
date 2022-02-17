#include <ThingSpeak.h>
#include <WiFiClient.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>

/*
 * This function implements the communication with the Thingspeak service
 */
bool thingspeak_call(char* api_key, char *api_additional, H32_Measurements &measurements, unordered_map<char *, double> &additional_data) {
  WiFiClient  client;

  int myChannelNumber = atoi(api_additional);
  myChannelNumber = myChannelNumber == 0 ? 1 : myChannelNumber;

  debug_println("Thingspeak API call");

  ThingSpeak.begin(client);  // Initialize ThingSpeak

  // set the fields with the values
  ThingSpeak.setField(1, (float)measurements.getTemperature());
  ThingSpeak.setField(2, (float)measurements.getHumidity());
  ThingSpeak.setField(3, (float)measurements.getBatV());
  ThingSpeak.setField(4, (float)measurements.getExtV());

  int x = ThingSpeak.writeFields(myChannelNumber, api_key);
  if(x == 200){
    debug_println("Channel update successful.");
    return true;
  } else{
    debug_println("Problem updating channel. HTTP error code " + String(x));
    return false;
  }
}

/*
 * Small helper function that creates a json from the data
 */
void create_json(JsonObject &doc, H32_Measurements &measurements, unordered_map<char *, double> &additional_data) {

  doc["Temperature"][0]["value"] = measurements.getTemperature();
  doc["Humidity"][0]["value"] = measurements.getHumidity();
  doc["Battery Voltage"][0]["value"] = measurements.getBatV();
  doc["External Voltage"][0]["value"] = measurements.getExtV();

  for(const auto & res: additional_data) {
     debug_print("Additional: ");
     debug_print(res.first);
     debug_print(" - ");
     debug_println(res.second);
    doc[res.first][0]["value"] = res.second;
  }
}

/*
 * This function implements the communication with the IOTPlotter service
 */
bool iotplotter_call(char* api_key, char *api_additional, H32_Measurements &measurements, unordered_map<char *, double> &additional_data) {
  WiFiClient client;
  HTTPClient httpClient;

  // Create the json needed for the service
  StaticJsonDocument<json_doc_size> doc;
  JsonObject object = doc.createNestedObject("data");
  create_json(object, measurements, additional_data);

  char json[json_doc_size];
  serializeJson(doc, json);

  debug_println("IOTPlotter JSON");
  debug_println(json);

  // Send the data with the needed header settings
  String serviceURL = "http://iotplotter.com/api/v2/feed/" + String(h32_config.api.additional);
  httpClient.begin(serviceURL);
  httpClient.addHeader("Content-Type", "application/x-www-form-urlencoded");
  httpClient.addHeader("api-key", h32_config.api.key);
  int http_response_code = httpClient.POST(json);

  debug_print(http_response_code);
  debug_print(" ");
  debug_println(httpClient.getString());

  // Disconnect
  httpClient.end();
   
  return http_response_code == 200;
}

/*
 * This function implements the communication with the IOTPlotter service
 */
const uint8_t mqtt_retries = 3;
bool mqtt_call(H32_Measurements &measurements, unordered_map<char *, double> &additional_data) {
  WiFiClient  client;
  PubSubClient mqttClient(client);

  debug_println("MQTT");

  mqttClient.setServer(h32_config.mqtt.server, h32_config.mqtt.port);

  StaticJsonDocument<json_doc_size> doc;

  // Try to connect for "mqtt_retries" times. If successful, publish data as json
  for(int i = 0; i < mqtt_retries; i++) {
    if(mqttClient.connect(h32_config.name, h32_config.mqtt.user, h32_config.mqtt.passwd)) {

      debug_println("Connected to MQTT");
      JsonObject object = doc.to<JsonObject>();
      create_json(object, measurements, additional_data);

      char json[json_doc_size];
      serializeJson(doc, json);

      debug_println("MQTT JSON");
      debug_println(json);

      return mqttClient.publish (h32_config.mqtt.topic, json);
    }
    delay(100);
  }

  return false;
}
