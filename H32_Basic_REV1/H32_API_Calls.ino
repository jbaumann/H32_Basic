#include <ThingSpeak.h>
#include <WiFiClient.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>

/*
 * This function implements the communication with the Thingspeak service
 */
bool thingspeak_call(char* api_key, char *api_additional, double temperature, double humidity, double bat_v, double ext_v) {
  WiFiClient  client;

  int myChannelNumber = atoi(api_additional);
  myChannelNumber = myChannelNumber == 0 ? 1 : myChannelNumber;

  debug_println("Thingspeak API call");

  ThingSpeak.begin(client);  // Initialize ThingSpeak

  // set the fields with the values
  ThingSpeak.setField(1, (float)temperature);
  ThingSpeak.setField(2, (float)humidity);
  ThingSpeak.setField(3, (float)bat_v);
  ThingSpeak.setField(4, (float)ext_v);

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
String create_json(double temperature, double humidity, double bat_v, double ext_v) {
  String json;
  json += "{ \"Temperature\": [ {\"value\": " + String(temperature) + " } ],";
  json += "\"Humidity\": [ {\"value\": " + String(humidity) + " } ],";
  json += "\"Battery Voltage\": [ {\"value\": " + String(bat_v) + " } ],";
  json += "\"External Voltage\": [ {\"value\": " + String(ext_v) + " } ] }";
  return json;
}

/*
 * This function implements the communication with the IOTPlotter service
 */
bool iotplotter_call(char* api_key, char *api_additional, double temperature, double humidity, double bat_v, double ext_v) {
  WiFiClient client;
  HTTPClient httpClient;

  // Create the json needed for the service
  String json = "{\"data\":";
  json += create_json(temperature, humidity, bat_v, ext_v);
  json += "}";
  debug_println(json);

  // Send the data with the needed header settings
  String serviceURL = "http://iotplotter.com/api/v2/feed/" + String(h32_config.api.additional);
  httpClient.begin(serviceURL);
  httpClient.addHeader("Content-Type", "application/x-www-form-urlencoded");
  httpClient.addHeader("api-key", h32_config.api.key);
  int http_response_code = httpClient.POST(json);

  debug_println(http_response_code);
  debug_println(httpClient.getString());

  // Disconnect
  httpClient.end();
   
  return http_response_code == 200;
}

/*
 * This function implements the communication with the IOTPlotter service
 */
const uint8_t mqtt_retries = 3;
bool mqtt_call(double temperature, double humidity, double bat_v, double ext_v) {
  WiFiClient  client;
  PubSubClient mqttClient(client);

  debug_println("MQTT");

  mqttClient.setServer(h32_config.mqtt.server, h32_config.mqtt.port);

  // Try to connect for "mqtt_retries" times. If successful, publish data as json
  for(int i = 0; i < mqtt_retries; i++) {
    if(mqttClient.connect(h32_config.name, h32_config.mqtt.user, h32_config.mqtt.passwd)) {
      debug_println("Connected to MQTT");
      return mqttClient.publish (h32_config.mqtt.topic, create_json(temperature, humidity, bat_v, ext_v).c_str());
      break;
    }
    delay(100);
  }

  return false;
}
