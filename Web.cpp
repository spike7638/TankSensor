#include "TouchAndSense.h"
#include "Web.h"
#include "Persistence.h"
#include <ArduinoJson.h>


/////////////////////////////////////////////////////////////////
// Web handling setup: connect to a server, create your own mDNS address, etc.
// First step is to get rid of all the pages that AsyncFsWebServer creates.
/////////////////////////////////////////////////////////////////

/* 
 * AsyncFsWebServer does a LOT of things all at once. 
 * 1. It sets up a tiny filesystem on the ESP32 (in our case, using the LittleFS library)
 * 1.1 It saves some wifi credentials in another part of the ESP32's persistent memory
 * 2. It sets up a webserver using a local Wifi network (using the stored credentials from 1.1), except...
 * 3. If there's no local Wifi, or it can't connect to it, it sets up its own wifi network, and a reduced web-interface, a "captive portal"
 *    You can use this captive portal to enter new credentials, which it can then store and use to retry the connection process
 *.   You can also, through a process I don't understand, use it to do OTA updates of the stored program on the ESP-32.
 * 4. In either case, it optionally provides a webpage that lets you edit files in the tiny filesystem, or upload new ones, or delete existing ones.
 * 5. Also, the "content" of the webserver consists of paths resolved in the code (with things like "server.on("/help", <do something>)") AND the content 
 *    of the tiny filesystem, which can include html pages that contain embedded javascript, etc. That makes editing the appearance/function of the 
 *    resulting website much easier, as no new compilation of this program is required in general. 
 * 6. Finally, to let users reach this webserver, we need to give it a name, like "google.com", that can be found by browsers. That's a function typically provided
 *    by a domain name server (DNS), where names get looked up and turned into IP addresses like 137.22.14.3, etc. AsyncFsWebServer lets us establish a name (our default
 *    is esp32-tanksensor, defined in Persistence.cpp) which it shares with the world using "multicast DNS" or mDNS. A browser on the same Wifi network can look for the 
 *    site "esp32-tanksensor.local" and it'll get hooked up to our server. (Do some reading about mDNS --- it's pretty cool). Of course, we let you choose a different name
 *    if you like. 
 *
 * In our particular use of this library, the tiny filesystem contains minimal stuff (about 5 files), and many of the automatically-installed handlers 
 * (stuff with "server.on("<some path>", <do something>)") are of no use to us, and we want to get rid of them. The "server.init()" procedure creates this
 * stuff, and our first task is to get rid of all of it and then re-create the few bits we want. 
 *
 * The things that the tiny filesystem contains are webpages for 
 * (a) Adjusting the sensor's low- and hi-limit settings, and critical threshold, etc.
 * (b) Adjusting system settings: the network name (DNS name) and the sensor name ("Fuel tank" or "Holding tank")
 * (c) Adjusting general settings: you can choose a new password, or reset the entire system to its default settings. 
 *
 * There's also a main page (index.htm) that shows the current sensor readings, and provides links to the three "adjustment" pages listed above.
 * Finally, there's a config subdirectory containing a json file that seems to be unused, but AsyncFsWebServer creates it by default. 
 */

// Something for all procedures to be able to refer to; actual value is defined in "init" procedure
static AsyncFsWebServer* server;

////////////////////////////  HTTP Request Handlers  ////////////////////////////////////

// Show the tank fullness
// Only used if someone goes to esp32-tanksensor.local/getDefault
void getDefaultValue (AsyncWebServerRequest *request) {
  String Part1 = "<!DOCTYPE html><html><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\"></head><body><h1>"  + getSensorName() + String(" level: ");
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

// get the name of the sensor; the "2" at the end is to avoid conflicts with Persistence module
void getSensorName2 (AsyncWebServerRequest *request) {
  // report: current sensor value
  #define OUTPUT_STRING_SIZE 200
  JsonDocument doc;
  char output[OUTPUT_STRING_SIZE];
  doc["sensorName"] = getSensorName(); // getSensorName from Persistence.cpp
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

/* Start up the LittleFS filesystem, and list the current contents so that we see what's there. */
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

void webInit(bool show_editor) {
  // delay(100);
  // Try to connect to stored SSID, start AP if fails after timeout
  // This "temporary" AP will have a name like ESP-3F28AC44,
  // and password 123456789

  /* Start up a webserver with associated filesystem LittleFS,
   * and a DNS name from the Persistence module (with .local appended)
   */
  String s = getNetworkName();
  static AsyncFsWebServer sserver(80, FILESYSTEM, s.c_str());  
  Serial.print("Starting up with DNS name: ");
  Serial.println( s + String(".local !"));
  // Make server available to all procedures in this module
  server = &sserver;

  /* Try to connect to Wifi with stored credentials. If this fails,
   * create a Wifi network with a unique name like EPS-A6742B,
   * for which the webserver will be a captive portal: any request 
   * takes you to a "setup" page where you enter the ssid and password (the "credentials")
   * for the Wifi network you want to use, and when you click a button to connect to 
   * that network, these new credentials are stored for all subsequent startups.
   */
  
  char default_ssid[23];
  snprintf(default_ssid, 23, "ESP-%08X", (uint32_t)ESP.getEfuseMac());
  IPAddress myIP = sserver.startWiFi(15000, default_ssid, "123456789");
  WiFi.setSleep(WIFI_PS_NONE);
 // FILESYSTEM INIT, and show current content
  startFilesystem();

  // Add custom page handlers to webserver
  sserver.on("/getDefault", HTTP_GET, getDefaultValue);
  sserver.on("/setSensorSettingsForm", HTTP_POST, handleSensorSettingsForm);
  sserver.on("/setSystemSettingsForm", HTTP_POST, handleSystemSettingsForm);
  sserver.on("/setGeneralSettingsForm", HTTP_POST, handleGeneralSettingsForm);
  
  sserver.on("/getSettings", HTTP_GET, getSettings);
  sserver.on("/getSensor", HTTP_GET, getSensor);
  sserver.on("/getSensorName", HTTP_GET, getSensorName2);

  // Enable ACE FS file web editor and add FS info callback function
  if (show_editor) {
    sserver.enableFsCodeEditor();
  }
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
