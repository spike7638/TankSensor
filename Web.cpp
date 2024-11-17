#include "TouchAndSense.h"
#include "Web.h"
#include "Persistence.h"
#include "Display.h"
#include "Tank.h"
#include <ArduinoJson.h>
#include "nvs_flash.h"

/////////////////////////////////////////////////////////////////
// Web handling setup: connect to a server, create your own mDNS address, etc.
// First step is to get rid of all the pages that AsyncFsWebServer creates.
/////////////////////////////////////////////////////////////////

/* 
 * AsyncFsWebServer does a LOT of things all at once. 
 * 1. It sets up a tiny filesystem on the ESP32 (in our case, using the LittleFS library)
 * 1.1 It saves some wifi credentials in another part of the ESP32's persistent memory ("nvs" = non-volatile storage)
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

// Give a JSON summary of the current tank level
void getLevel (AsyncWebServerRequest *request) {
  // report: current tank level
  #define OUTPUT_STRING_SIZE 200
  JsonDocument doc;
  char output[OUTPUT_STRING_SIZE];
  doc["tankLevel"] = getTankLevelPercent();
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

/////////////// Server setup stuff ////////////////////////
void setTaskWdt(uint32_t timeout) {
    #if defined(ESP32)
    #if ESP_ARDUINO_VERSION_MAJOR > 2
    esp_task_wdt_config_t twdt_config = {
        .timeout_ms = timeout,
        .idle_core_mask = (1 << portNUM_PROCESSORS) - 1,    // Bitmask of all cores
        .trigger_panic = false,
    };
    ESP_ERROR_CHECK(esp_task_wdt_reconfigure(&twdt_config));
    #else
    ESP_ERROR_CHECK(esp_task_wdt_init(timeout/1000, 0));
    #endif
    #endif
}

void notFound(AsyncWebServerRequest *request) {
    request->send(404, "text/plain", "Not found");
    log_debug("Resource %s not found\n", request->url().c_str());
}

void handleSetup(AsyncWebServerRequest *request) {
    // Changed array name to match SEGGER Bin2C output
    AsyncWebServerResponse *response = request->beginResponse_P(200, "text/html", (uint8_t*)_acsetup_min_htm, sizeof(_acsetup_min_htm));
    response->addHeader("Content-Encoding", "gzip");
    response->addHeader("X-Config-File", ESP_FS_WS_CONFIG_FILE);
    request->send(response);
}

void getStatus(AsyncWebServerRequest *request) {
    JSON_DOC(256);
    //doc["firmware"] = m_version;
    doc["mode"] =  WiFi.status() == WL_CONNECTED ? ("Station (" + WiFi.SSID()) +')' : "Access Point";
    doc["ip"] = (WiFi.status() == WL_CONNECTED) ? WiFi.localIP().toString() : WiFi.softAPIP().toString();
    doc["path"] = String(ESP_FS_WS_CONFIG_FILE).substring(1);   // remove first '/'
    doc["liburl"] = LIB_URL;
    String reply;
    serializeJson(doc, reply);
    request->send(200, "application/json", reply);
}

void doWifiConnection(AsyncWebServerRequest *request) {
    String ssid, pass;
    IPAddress gateway, subnet, local_ip;
    bool config = false,  newSSID = false;

    if (request->hasArg("ip_address") && request->hasArg("subnet") && request->hasArg("gateway")) {
        gateway.fromString(request->arg("gateway"));
        subnet.fromString(request->arg("subnet"));
        local_ip.fromString(request->arg("ip_address"));
        config = true;
    }

    if (request->hasArg("ssid"))
        ssid = request->arg("ssid");

    if (request->hasArg("password"))
        pass = request->arg("password");

    if (request->hasArg("newSSID")) {
        newSSID = true;
    }

    /*
    *  If we are already connected and a new SSID is needed, once the ESP will join the new network,
    *  /setup web page will no longer be able to communicate with ESP and therefore
    *  it will not be possible to inform the user about the new IP address.
    *  Inform and prompt the user for a confirmation (if OK, the next request will force disconnect variable)
    */
    if (WiFi.status() == WL_CONNECTED && !newSSID) {
        char resp[512];
        snprintf(resp, sizeof(resp),
            "ESP is already connected to <b>%s</b> WiFi!<br>"
            "Do you want close this connection and attempt to connect to <b>%s</b>?"
            "<br><br><i>Note:<br>Flash stored WiFi credentials will be updated.<br>"
            "The ESP will no longer be reachable from this web page "
            "due to the change of WiFi network.<br>To find out the new IP address, "
            "check the serial monitor or your router.<br></i>",
            WiFi.SSID().c_str(), ssid.c_str()
        );
        request->send(200, "application/json", resp);
        return;
    }

    if (request->hasArg("persistent")) {
        if (request->arg("persistent").equals("false")) {
            WiFi.persistent(false);
            #if defined(ESP8266)
                struct station_config stationConf;
                wifi_station_get_config_default(&stationConf);
                // Clear previous configuration
                memset(&stationConf, 0, sizeof(stationConf));
                wifi_station_set_config(&stationConf);
            #elif defined(ESP32)
                wifi_config_t stationConf;
                esp_wifi_get_config(WIFI_IF_STA, &stationConf);
                // Clear previous configuration
                memset(&stationConf, 0, sizeof(stationConf));
                esp_wifi_set_config(WIFI_IF_STA, &stationConf);
            #endif
        }
        else {
            // Store current WiFi configuration in flash
            WiFi.persistent(true);
            #if defined(ESP8266)
                struct station_config stationConf;
                wifi_station_get_config_default(&stationConf);
                // Clear previous configuration
                memset(&stationConf, 0, sizeof(stationConf));
                os_memcpy(&stationConf.ssid, ssid.c_str(), ssid.length());
                os_memcpy(&stationConf.password, pass.c_str(), pass.length());
                wifi_set_opmode(STATION_MODE);
                wifi_station_set_config(&stationConf);
            #elif defined(ESP32)
                wifi_config_t stationConf;
                esp_wifi_get_config(WIFI_IF_STA, &stationConf);
                // Clear previuos configuration
                memset(&stationConf, 0, sizeof(stationConf));
                memcpy(&stationConf.sta.ssid, ssid.c_str(), ssid.length());
                memcpy(&stationConf.sta.password, pass.c_str(), pass.length());
                esp_err_t err = esp_wifi_set_config(WIFI_IF_STA, &stationConf);
                if (err) {
                    log_error("Set WiFi config: %s", esp_err_to_name(err));
                }
            #endif
        }
    }

    // Connect to the provided SSID
    if (ssid.length() && pass.length()) {
        setTaskWdt(TIMEOUT + 1000);
        WiFi.mode(WIFI_AP_STA);

        // Manual connection setup
        if (config) {
            log_info("Manual config WiFi connection with IP: %s", local_ip.toString().c_str());
            if (!WiFi.config(local_ip, gateway, subnet)) {
                log_error("STA Failed to configure");
            }
        }
        
        Serial.println("");
        Serial.printf("Connecting to %s\n", ssid.c_str());
        WiFi.begin(ssid.c_str(), pass.c_str());

        if (WiFi.status() == WL_CONNECTED && newSSID) {
            log_info("Disconnect from current WiFi network");
            WiFi.disconnect();
            delay(10);
        }

        uint32_t beginTime = millis();
        while (WiFi.status() != WL_CONNECTED) {
            delay(250);
            Serial.print("*");
            #if defined(ESP8266)
            ESP.wdtFeed();
            #else
            esp_task_wdt_reset();
            #endif
            if (millis() - beginTime > TIMEOUT) {
                request->send(408, "text/plain", "<br><br>Connection timeout!<br>Check password or try to restart ESP.");
                delay(100);
                Serial.println("\nWiFi connect timeout!");;
                break;
            }
        }
        // reply to client
        if (WiFi.status() == WL_CONNECTED) {
            IPAddress ip = WiFi.localIP();
            Serial.print(F("\nConnected to Wifi! IP address: "));
            Serial.println(ip);
            String serverLoc = F("http://");
            for (int i = 0; i < 4; i++)
                serverLoc += i ? "." + String(ip[i]) : String(ip[i]);
            serverLoc += "/setup";

            char resp[256];
            snprintf(resp, sizeof(resp),
                "ESP successfully connected to %s WiFi network. <br><b>Restart ESP now?</b>"
                "<br><br><i>Note: disconnect your browser from ESP AP and then reload <a href='%s'>%s</a></i>",
                ssid.c_str(), serverLoc.c_str(), serverLoc.c_str()
            );
            log_debug("%s", resp);
            request->send(200, "application/json", resp);
        }
    }
    setTaskWdt(8000);
    request->send(401, "text/plain", "Wrong credentials provided");
}

wifi_mode_t getWifiMode()
{
  
  IPAddress zeroIP = IPAddress(0,0,0,0);
  IPAddress stationIP = WiFi.networkID();
  IPAddress accessIP = WiFi.softAPIP();
  
  boolean isStation = (stationIP != zeroIP);
  boolean isAccessPoint = (accessIP != zeroIP);
  if (isStation && isAccessPoint)
    return WIFI_AP_STA;
  else if (isStation)
    return WIFI_STA;
  else if (isAccessPoint)
    return WIFI_AP;
  else
    return WIFI_MODE_NULL;
    
}

/* 
** Check whether we're successfully in station mode, connected to an intended
** wireless network. 
** If NOT, then set up an access point, and a captive portal, and skip anything else. 
**
** If SO, 
**.  Do much of the stuff done in AsyncFsWebServer:init,
**   but with fewer default webpages included. 
**   In particular, if "include_setup" is true, add a /setup page
**   to allow you to mess with the wifi settings. And then run 
**   the mdns server to let you access the pages with some name like eps-sensor/index.htm etc. 
*/
void alternativeInit(bool powerMode)
{
  server->reset(); // get rid of all the existing server handlers.
  //using namespace std::placeholders;
  bool include_setup = powerMode;
  wifi_mode_t q = getWifiMode(); 

  if (q != WIFI_STA) { // The bad case --- had to set up an access point
    Serial.println("Couldn't connect; Adding access point");

    displayMessage("Could not connect", 1.5);
    displayMessage("Starting Access Point", 1.5);
    displayMessage("name: " + WiFi.softAPSSID(), 1.5);
/*    auto handler = server->on("/setup", HTTP_GET, handleSetup);
    server->on("/getStatus", HTTP_GET, getStatus);
    server->on("/connect", HTTP_POST, doWifiConnection);
  //  server->on("/*", HTTP_GET, handleSetup);
    include_setup = true;
    */

  }

  if (include_setup) {
    Serial.println("Adding /setup page");
    displayMessage("Adding /setup page", 1);
    auto handler = server->on("/setup", HTTP_GET, handleSetup);
    server->on("/getStatus", HTTP_GET, getStatus);
    server->on("/connect", HTTP_POST, doWifiConnection);
    server->onNotFound( notFound);
    server->serveStatic("/", LittleFS, "/").setDefaultFile("index.htm");
  }

  displayMessage("Connected to WiFi", 1.5);
  displayMessage("name: " + WiFi.SSID(), 1.5);
  displayMessage("Visit...", 1.0);
  displayMessage(getNetworkName() + String(".local"), 2.0);

  
  //server->on("*", HTTP_HEAD, std::bind(&AsyncFsWebServer::handleFileName, this, _1));
  server->onNotFound( notFound);
  server->serveStatic("/", LittleFS, "/").setDefaultFile("index.htm");
  

  MDNS.end();
  
  Serial.println("Starting MDNS with name: " + getNetworkName());
  
  if ((q != WIFI_STA)) {
    if (!MDNS.begin(getNetworkName().c_str())){
        log_error("MDNS responder started");
    }
  }
  else {
    if (!MDNS.begin(getNetworkName().c_str())){
        log_error("MDNS responder started");
    }
  }
 
  MDNS.addService("http", "tcp", 80);
  MDNS.setInstanceName("async-fs-webserver");
    
}



///////////// Main initialization ///////////////////
void webInit(bool powerMode) {
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
   * create a Wifi network with a unique name like ESP-A6742B.
   * If you're in power mode, then 
   * the webserver will be a captive portal: any request 
   * takes you to a "setup" page where you enter the ssid and password (the "credentials")
   * for the Wifi network you want to use, and when you click a button to connect to 
   * that network, these new credentials are stored for all subsequent startups.

   * If you're NOT in power mode, then just operate as normal, with all the web pages showing up
   * on the ESP-XXXXXX network (but not /setup or /edit).
   */
  
  char default_ssid[23];
  snprintf(default_ssid, 23, "ESP-%08X", (uint32_t)ESP.getEfuseMac());
  IPAddress myIP = sserver.startWiFi(15000, default_ssid, "123456789");
  WiFi.setSleep(WIFI_PS_NONE);
 // FILESYSTEM INIT, and show current content
  startFilesystem();
  sserver.init(); 
  alternativeInit(powerMode); // initialize web system
  Serial.print(F("ESP Web Server started on IP Address: "));
  Serial.println(myIP);
  Serial.println(F(
    "Open /edit page to edit custom webserver source files. You probably don't want to do this,\n"
    "and it'll only work if you held down the 'touch' connection during the first few seconds of startup.\n\n"
    "If we're unable to connect to a local Wifi network, then we start our own, with a name like\n"
    "ESP-xxxxxxx, where each x is a digit or letter. Connecting to this will\n"
    "cause it to open a /setup page to let you configure the WiFi connection.\n"
  ));

  // Add custom page handlers to webserver
  sserver.on("/getDefault", HTTP_GET, getDefaultValue);
  sserver.on("/setSensorSettingsForm", HTTP_POST, handleSensorSettingsForm);
  sserver.on("/setSystemSettingsForm", HTTP_POST, handleSystemSettingsForm);
  sserver.on("/setGeneralSettingsForm", HTTP_POST, handleGeneralSettingsForm);
  
  sserver.on("/getSettings", HTTP_GET, getSettings);
  sserver.on("/getSensor", HTTP_GET, getSensor);
  sserver.on("/getLevel", HTTP_GET, getLevel);
  sserver.on("/getSensorName", HTTP_GET, getSensorName2);

  // Enable ACE FS file web editor and add FS info callback function
  if (powerMode) {
    sserver.enableFsCodeEditor();
  }
  sserver.setFsInfoCallback( [](fsInfo_t* fsInfo) {
	fsInfo->fsName = "LittleFS";
	fsInfo->totalBytes = LittleFS.totalBytes();
	fsInfo->usedBytes = LittleFS.usedBytes();  
  });  
}

