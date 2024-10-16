
//#include "ArduinoJson/Document/JsonDocument.hpp"
#include "TouchAndSense.h"
#include "Web.h"
#include "Persistence.h"
#include <ArduinoJson.h>


/////////////////////////////////////////////////////////////////
// Web handling setup: use mDNS to register a readable name, etc.
/////////////////////////////////////////////////////////////////

#define NAME_SIZE 200
static char nameBuffer[NAME_SIZE];
static AsyncFsWebServer* server;

////////////////////////////  HTTP Request Handlers  ////////////////////////////////////

// Show the tank fullness
// To do: add the tank name
void getDefaultValue (AsyncWebServerRequest *request) {
  String Part1 = "<!DOCTYPE html><html><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\"></head><body><h1>Tank level: ";
  String Part2 = String(getSenseValue());
  String Part3 = "</h1></body></html>";

request->send(200, "text/html", Part1 + Part2 + Part3); 
}

// Give a JSON summary of the current sense value
void getSensor (AsyncWebServerRequest *request) {
  // report: current sensor value
  #define OUTPUT_STRING_SIZE 200
  JsonDocument doc;
  char output[OUTPUT_STRING_SIZE];
  doc["senseValue"] = getSenseValue();
  serializeJson(doc, output, OUTPUT_STRING_SIZE);     
  request->send(200, "text/json", output);
}

// Give a JSON summary of the settings
void getSettings (AsyncWebServerRequest *request) {
  // report: LO, HIGH, THRESHOLD, DIR, networkName, sensorName, (password?)
  #define OUTPUT_STRING_SIZE 200
  JsonDocument doc;
  char output[OUTPUT_STRING_SIZE];
 // obj["password"] = getPassword(); // omit this for obvious reasons
  doc["lowerLimit"] = getLowerLimit();
  doc["upperLimit"] = getUpperLimit();
  doc["criticalValue"] = getCriticalValue();
  doc["criticalDirection"] = getCriticalDirection();
  doc["networkName"] = getNetworkName();
  doc["sensorName"] = getSensorName();

  serializeJson(doc, output, OUTPUT_STRING_SIZE);     
  request->send(200, "text/json", output);
}

// If there's a password in the request, and it's the right one, then true; else false. 
bool checkPassword(AsyncWebServerRequest *request){
  Serial.print("check PW; password is:");
  Serial.println(request->arg("password"));
  Serial.print("check PW; current password is:");
  Serial.println(getPassword());
  if(request->hasArg("password")) {
    if (request->arg("password") == getPassword()) {
      return true; 
    };
  }
  return false;
}

void handleSensorSettingsForm(AsyncWebServerRequest *request) {
  String reply;
  Serial.println("Sensor Settings Form submitted");
  if (checkPassword(request)) {
    setLowerLimit(request->arg("lowerLimit").toInt());      
    setUpperLimit(request->arg("upperLimit").toInt());      
    setCriticalValue(request->arg("criticalValue").toInt());      
    setCriticalDirection(request->arg("criticalDirection").toInt());
    reply += "Settings updated!";
    request->send(200, "text/plain", reply);
  } 
  else
  {
  request->send(200, "text/plain", "Incorrect password; no settings changed.");
  }
  Serial.println(reply);
}

void handleSystemSettingsForm(AsyncWebServerRequest *request) {
  String reply;
  Serial.println("System Settings Form submitted");
  if (checkPassword(request)) {
    setSensorName(request->arg("sensorName"));      
    reply += "Settings updated!";
    request->send(200, "text/plain", reply);
    if (request->arg("networkName") != getNetworkName()){
      setNetworkName(request->arg("networkName"));      
      request->send(200, "text/plain", "Network name reset");
      setup();
    }
  } 
  else
  {
  request->send(200, "text/plain", "Incorrect password; no settings changed.");
  }
  Serial.println(reply);
}

void handleGeneralSettingsForm(AsyncWebServerRequest *request) {
  String reply;
  bool resetCheck = false;
  Serial.println("General Settings Form submitted");
  if (checkPassword(request)) {
    if (request->arg("newPassword") == ""){
      reply += "Empty password not allowed; password unchanged!";
    }
    else
    {
      Serial.println("proposed new PW:" + request->arg("newPassword") + ":");
      setPassword(request->arg("newPassword"));
      reply += "Password changed!";      
    }
    if (request->arg("reset") == "reset") {
      Serial.println("about to do general reset");
  
      persistenceReset();
      resetCheck = true; 
      reply += "System reset (which overrides any password change)!";
    }
    request->send(200, "text/plain", reply);
    if (resetCheck)  setup();
  } 
  else
  {
    request->send(200, "text/plain", "Incorrect password; no settings changed.");
  }
  Serial.println(reply);
}

////////////////////////////////  Filesystem  /////////////////////////////////////////
void listDir(fs::FS &fs, const char * dirname, uint8_t levels){
  Serial.printf("\nListing directory: %s\n", dirname);
  File root = fs.open(dirname, "r");
  if (!root) {
    Serial.println("- failed to open directory");
    return;
  }
  if (!root.isDirectory()) {
    Serial.println(" - not a directory");
    return;
  }
  File file = root.openNextFile();
  while (file) {
    if (file.isDirectory()) {
      if (levels) {
        #ifdef ESP32
          listDir(fs, file.path(), levels - 1);
        #elif defined(ESP8266)
          listDir(fs, file.fullName(), levels - 1);
        #endif
      }
    } else {
      Serial.printf("|__ FILE: %s (%d bytes)\n",file.name(), file.size());
    }
    file = root.openNextFile();
  }
}

bool startFilesystem() {
  if (FILESYSTEM.begin()){
    listDir(FILESYSTEM, "/", 1);
    return true;
  }
  else {
      Serial.println("ERROR on mounting filesystem. It will be reformatted!");
      FILESYSTEM.format();
      ESP.restart();
  }
  return false;
}

void webInit() {
 // delay(100);
  // Try to connect to stored SSID, start AP if fails after timeout
  // BTW, AsyncFsWebServer starts up mDNS, so we don't need to.
  String s = getNetworkName();
  s.toCharArray(nameBuffer,  NAME_SIZE);
  static AsyncFsWebServer sserver(80, LittleFS, nameBuffer);  
  Serial.print("Starting up with network name: ");
  Serial.println( getNetworkName());
  server = &sserver;

  IPAddress myIP = sserver.startWiFi(15000, "DCFBoat", "Prudence" );

 // FILESYSTEM INIT
  startFilesystem();

  // Add custom page handlers to webserver
  sserver.on("/getDefault", HTTP_GET, getDefaultValue);
  sserver.on("/setSensorSettingsForm", HTTP_POST, handleSensorSettingsForm);
  sserver.on("/setSystemSettingsForm", HTTP_POST, handleSystemSettingsForm);
  sserver.on("/setGeneralSettingsForm", HTTP_POST, handleGeneralSettingsForm);
  
  sserver.on("/getSettings", HTTP_GET, getSettings);
  sserver.on("/getSensor", HTTP_GET, getSensor);

  // Enable ACE FS file web editor and add FS info callback function
  sserver.enableFsCodeEditor();

  sserver.setFsInfoCallback( [](fsInfo_t* fsInfo) {
	fsInfo->fsName = "LittleFS";
	fsInfo->totalBytes = LittleFS.totalBytes();
	fsInfo->usedBytes = LittleFS.usedBytes();  
  });
  sserver.init();
  Serial.print(F("ESP Web Server started on IP Address: "));
  Serial.println(myIP);
  Serial.println(F(
    "Open /setup page to configure optional parameters.\n"
    "Open /edit page to view, edit or upload example or your custom webserver source files."
  ));
}
