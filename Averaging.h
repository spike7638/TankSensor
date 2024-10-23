#pragma once

#ifndef Averaging_h
#define Averaging_h
/////////////////////////////////////////////////////////////////

#include "Arduino.h"

/*
Average the readings from the sensor: keep some number, N_AVERAGE, of readings,
and for each new reading, replace the oldest one with the new one and recompute the average.

 */

// how many readings to average to get the current value-to-display
#define N_AVERAGE 100

void averageInit(); // initialize with a bunch of "0" readings
int updateAverage(int reading);


/////////////////////////////////////////////////////////////////
#endif
/////////////////////////////////////////////////////////////////
