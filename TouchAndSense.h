#pragma once

#ifndef TOUCH_AND_SENSE_h
#define TOUCH_AND_SENSE_h

#include "Arduino.h"
#include "hal/adc_types.h"

/*
** Detect inputs on two pins --- one to see if someone is touching an exposed piece of metal connected to pin 32,
** the other to detect a DC voltage between pin 33 and ground. 
** If there's a debugging LED (with appropriate resistor) connected to pin 2 and ground, 
** it will turn on/off depending on the sensed DC voltage.
*/

/////////////////////////////////////////////////////////////////
// set pin numbers
const int touchPin = T8;  // Physical pin 32; "T8" is a symbolic value associated to "touch input 8"

const int sensePin = T9;  // Physical pin 33; "T9" happens to also be one of the analog-to-digital-conversion pins
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