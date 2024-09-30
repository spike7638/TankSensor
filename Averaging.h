#pragma once

#ifndef Averaging_h
#define Averaging_h
/////////////////////////////////////////////////////////////////

#include "Arduino.h"




/*
Store, in persistent memory, 
* the wifi SSID and password, 
* the low, threshold, and hi settings for the sensor
* the sensor direction (threshold hi (+1) (e.g., holding tank) or lo (-1) (e.g., fuel))
* a password for the "making-changes" webpage.
* the name to use for the network
* the name to use for the sensor
*/

void averageInit();
int updateAverage(int reading);


/////////////////////////////////////////////////////////////////
#endif
/////////////////////////////////////////////////////////////////
