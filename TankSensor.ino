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
#include "Averaging.h"
#include <esp_task_wdt.h>


/* General notes 
This project will use a 4-20mA sensor to measure tank-fullness. There are several goals:
* Enter "empty" and "full" readings via the buttons on the EPS32 for initial setup
* In response to a touchpad touch, turn on the display and show a "level" gauge to give a visual sense of fullness
* Have that display turn off 19 seconds after the touuch is released
* Have the device connect to a local network with a default web page that shows how full the tank is
*
* ESP pinouts: https://sites.google.com/site/jmaathuis/arduino/lilygo-ttgo-t-display-esp32#h.p_uUP08r1njLqE
*
* The ESP-32 has two A AsyncFsWebServer units; the second one is used by the Wifi subsysten
* (see https://docs.espressif.com/projects/esp-idf/en/release-v4.4/esp32/api-reference/peripherals/adc.html)
* so we have to use the first (ADC1), which supports pins 32-39; we'll use pin 36 ("SVP" or "SENSOR_VP", where SVP and SVN are 
* names for a sensor measurement component that's now deprecated. Pin 36 is input only, with no pull-up or pull-down. 
*
* For touch-sensitive stuff,  we'll use pin 32, "TOUCH9" (which may be swapped with GPIO 33, depending on updates, so maybe we need to refer to 
* GPIO32 as T8 rather than T9)
*
* Temporarily, we'll use "touchPinWakeup" (pin 33) to signal that we should awake the display for 5 seconds, say. 
*
* For the low- and high-level setting feature, we'll use the left and right buttons, GPIO0 and GPIO35. 
*
* There's an LED temporarily attached to pin 2 to give feedback during development
*
* To avoid flicker, we'll use a sprite (see https://www.youtube.com/watch?v=sRGXQg7028A)
* 
* Also: OTA programming: https://lastminuteengineers.com/esp32-ota-updates-arduino-ide/
*/

// Wifi network to try initially

const char* defaultWifiSSID = "DCFBoat2";
const char* defaultWifiPassword = "Prudence";

const int serialSpeed = 115200;

bool awake = false; // true only when display should be on
int lastTouchMillis = -1; // the "millis" value when the touch-sensor was last touched
const int interval = 10000; // 10 seconds of awakeness after most recent touch

static bool show_editor = false;
String getDefaultWifiSSID(){
  return defaultWifiSSID;
}
String getDefaultWifiPassword(){
  return defaultWifiPassword;
}

void setup() {
  delay(1000); // enough time to switch to serial monitor after upload
  Serial.begin(serialSpeed); // 9600
  touchAndSenseInit();
  Serial.println("TouchAndSense init OK");
  // persistenceReset();  //uncomment only to reinitialize the system!
  persistenceInit();
  Serial.println("Persistence init OK");
  button_init();
  Serial.println("Button init OK");
  displayInit();
  Serial.println("Display init OK");
  averageInit();
  Serial.println("Average init OK");
  delay(1000);
  if (getTouchState()){
    show_editor = true;
    Serial.println("Editor enabled!");
  }
  webInit(show_editor);
  Serial.println("Web init OK");
  OTAInit();
  Serial.println("OTA init OK");
  displayActivate(true);
  displayText("PW: " + String(getPassword()));
  delay(4000);
  displayActivate(false);
}

void loop() {
  static int new_reading;

  button_loop();  // check whether the on-board buttons have been pressed
  if (getTouchState()){
    ArduinoOTA.handle();
  }
  int s = getSenseValue();
  new_reading = updateAverage(s+ random(-5, 6)); //FIX THIS TO REMOVE RANDOMNESS! 
  displayShowLevel(new_reading, getLowerLimit(), getUpperLimit(), getCriticalValue());

  if (getTouchState()) { // if the touch-pad is being touched
    awake = true; 
    lastTouchMillis = millis(); 
    displayActivate(true);
   // esp_task_wdt_reset();
  }
  else if (awake) {
    if (millis() - lastTouchMillis > interval) {
      awake = false; 
      lastTouchMillis = -1;
      displayActivate(false);
    //  esp_task_wdt_reset();
    }
  } 
  
}

#endif
