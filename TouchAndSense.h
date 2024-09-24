#pragma once

#ifndef TOUCH_AND_SENSE_h
#define TOUCH_AND_SENSE_h

#include "Arduino.h"
/////////////////////////////////////////////////////////////////
// set pin numbers
const int touchPin = T8; 

//temporary pins for development/debugging:
const int sensePin = T9;  // replace once we have the sensor installed, so that I read a sensor value from an ADC
const int ledPin = 2;

// For debiugging: a threshold sensor value to use in turning on/off an LED
#define DEBUG_THRESHOLD 40
#define TOUCH_THRESHOLD 40

void touchAndSenseInit();
int getSenseValue();
bool getTouchState();
/////////////////////////////////////////////////////////////////


#endif
/////////////////////////////////////////////////////////////////