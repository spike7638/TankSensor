/*
Code cleanup, testing, and documentation
   - .Define “power mode”
   - .“edit” and buttons and “setup” available in power mode
     xMake OTA active when touch-sensor is touched

Testing
.Start up in non-power mode; do buttons function? No, they don't, so OK. 
Startup in non-power mode bad wifi name; 
  .does portal appear when I go to esp-.../setup? (no)
  .Does edit appear (no)
  .Do other functions work? (yes)
Startup in power mode and bad wifi name
  .does portal appear when I go to esp-.../setup? (Yes, but you have to go to the ESP Wifi network first)
  .Does portal open and work? 
  .Does /edit work?

Startup with good wifi, no power mode
  .Is portal and edit hidden? yes
  .Is DNS name working? Yes
  .Is the sensor value display correct? Yes

Startup with good wifi, touch-sensor on: 
   .am I in power mode? yes
   .Does DNS name work? yes

*/
#pragma once

#ifndef TANK_SENSOR_ino
#define TANK_SENSOR_ino

#include <esp32-hal-adc.h>
#include "Web.h"
#include "Button2.h"
#include "ButtonHandling.h"
#include "OTA.h"
#include "Persistence.h"
#include "TouchAndSense.h"
#include "Tank.h"
#include "Averaging.h"
#include <esp_task_wdt.h>

// Wifi network to try initially
const char* defaultWifiSSID = "DCFBoat2";
const char* defaultWifiPassword = "Prudence";

const int serialSpeed = 115200;

bool awake = false; // true only when display should be on
int lastTouchMillis = -1; // the "millis" value when the touch-sensor was last touched
const int interval = 10000; // 10 seconds of awakeness after most recent touch
const int OTATimeout = 30000; // 30 seconds of opportunity to do over-the-air programming in power mode. 
static bool powerMode = false; // allow the editor and setup dialog to be accessed

String getDefaultWifiSSID(){
  return defaultWifiSSID;
}

String getDefaultWifiPassword(){
  return defaultWifiPassword;
}

void setup() {
  delay(1000); // enough time to switch to serial monitor after upload
  Serial.begin(serialSpeed); // 9600
  displayInit();
  displayActivate(true);
  Serial.println("Display init OK");
 // displayMessage("Display", 1);
  touchAndSenseInit();
  Serial.println("TouchAndSense init OK");
//  displayMessage("Touch/Sense", 1);
  // persistenceReset();  //uncomment only to reinitialize the system!
  persistenceInit();
  Serial.println("Persistence init OK");
//  displayMessage("Persistence", 1);
  button_init();
  Serial.println("Button init OK");
//  displayMessage("Buttons", 1);
  averageInit();
  Serial.println("Average init OK");
//  displayMessage("Averaging", 1);
  delay(1000);
  if (getTouchState()){
    powerMode = true;
    displayMessage("Power-mode on!", 1);
  }
  webInit(powerMode);
  Serial.println("Web init OK");
//  displayMessage("WebServer", 1);
/*  if (powerMode) {
    OTAInit();
    Serial.println("OTA init OK");
//  displayMessage("OTA", 1);
  }
  */
  displayText("PW: " + String(getPassword()));
  delay(4000);
  displayActivate(false);
}

void loop() {
  if (powerMode) button_loop();  // check whether the on-board buttons have been pressed
  
/*  if (powerMode && (millis() < OTATimeout)) {
    ArduinoOTA.handle();
  }
*/
  displayShowLevelGauge(getTankLevelPercent(), getCriticalLevelPercent(), getCriticalDirection());

  if (getTouchState()) { // if the touch-pad is being touched
    awake = true; 
    lastTouchMillis = millis(); 
    displayActivate(true);
  }
  else if (awake) {
    if (millis() - lastTouchMillis > interval) {
      awake = false; 
      lastTouchMillis = -1;
      displayActivate(false);
    }
  } 
  
}

#endif
