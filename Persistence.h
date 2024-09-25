#pragma once

#ifndef Persistence_h
#define Persistence_h
/////////////////////////////////////////////////////////////////

#include <Preferences.h>
#include "Arduino.h"


/*
Store, in persistent memory, 
* the wifi SSID and password, 
* the low, threshold, and hi settings for the sensor
* the sensor direction (threshold hi (e.g., holding tank) or lo (e.g., fuel))
* a password for the "making-changes" webpage.
*/

void persistenceInit();
void persistenceReset();

String getPassword();
int getLowerLimit();
int getUpperLimit();
int getCriticalValue();
int getCriticalDirection();
String getNetworkName();
String getSensorName();

void setPassword(String s);
void setNetworkName(String s);
void setSensorName(String s);
void setLowerLimit(int lim);
void setUpperLimit(int lim);
void setCriticalValue(int val);
void setCriticalDirection(int val);

/////////////////////////////////////////////////////////////////
#endif
/////////////////////////////////////////////////////////////////
