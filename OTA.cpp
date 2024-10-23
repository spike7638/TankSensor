#include "OTA.h"
#include "Persistence.h"

/////////////////////////////////////////////////////////////////
// OTA programming setup
/////////////////////////////////////////////////////////////////
/*
If the Wifi SSID/password in persistent memory work, use them. 
If not ... perhaps just do the remaining setup stuff 
and see whether that works OK with the rest of the code (although OTA won't be possible, of course).
*/


void OTAInit() 
{
   while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
  /*
  Assume that the wifi connection has already been made. 
  String ssid = getWifiSSID();
  String password = getWifiPassword();
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  delay(500);
  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Abandoning OTA setup.");
    WiFi.disconnect();
//    delay(5000);
//    ESP.restart();
*/
/*
    Serial.println("Trying Access-point mode, network-name 'sensor'. ");
    WiFi.disconnect();
    WiFi.config(IPAddress(199,168,1,1), IPAddress(10,0,1,1), IPAddress(255,255,255,0));
    WiFi.mode(WIFI_AP);
    WiFi.softAP("sensor");
    Serial.print("ESP32 IP as soft AP: ");
    Serial.println(WiFi.softAPIP());
    */
  /*}
  else {
    */
    /*
    ArduinoOTA
      .onStart([]() {
        String type;
        if (ArduinoOTA.getCommand() == U_FLASH)
          type = "sketch";
        else // U_SPIFFS
          type = "filesystem";

        // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
        Serial.println("Start updating " + type);
      })
      .onEnd([]() {
        Serial.println("\nEnd");
      })
      .onProgress([](unsigned int progress, unsigned int total) {
        Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
      })
      .onError([](ota_error_t error) {
        Serial.printf("Error[%u]: ", error);
        if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
        else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
        else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
        else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
        else if (error == OTA_END_ERROR) Serial.println("End Failed");
      });
      */

    ArduinoOTA.begin();
    Serial.println("Ready");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  /*} */
}

/////////////////////////////////////////////////////////////////