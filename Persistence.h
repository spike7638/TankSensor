#pragma once

#ifndef Persistence_h
#define Persistence_h
/////////////////////////////////////////////////////////////////

#include <Preferences.h>
#include "Arduino.h"


/*
* Store, in the ESP-32 persistent memory, 
* the wifi SSID and password, 
* the low, threshold, and hi settings for the sensor
* the sensor direction (threshold hi (+1) (e.g., holding tank) or lo (-1) (e.g., fuel))
* a password for the "making-changes" webpage.
* the name to use for the network
* the name to use for the sensor
*/

void persistenceInit();
void persistenceReset();  // make it look as if no data has been stored yet, so that "init" needs to be called. 
                          // Unused in the program, unless you put it into TankSensor.ino's "setup()" procedure for one compile-run loop, and then delete it.

String getPassword();
int getLowerLimit();
int getUpperLimit();
int getCriticalValue();
int getCriticalDirection();
String getNetworkName();
String getSensorName();
String getWifiSSID();
String getWifiPassword();

void setPassword(String s);
void setLowerLimit(int lim);
void setUpperLimit(int lim);
void setCriticalValue(int val);
void setCriticalDirection(int val);
void setNetworkName(String s);
void setSensorName(String s);
void setWifiSSID(String s);
void setWifiPassword(String s);

/////////////////////////////////////////////////////////////////
#endif
/////////////////////////////////////////////////////////////////
