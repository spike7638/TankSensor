#pragma once

#ifndef TOUCH_AND_SENSE_h
#define TOUCH_AND_SENSE_h

#include "Arduino.h"
#include "hal/adc_types.h"
/////////////////////////////////////////////////////////////////
// set pin numbers
const int touchPin = T8;  // Physcial pin 32

const int sensePin = T9;  // Physical pin 33
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