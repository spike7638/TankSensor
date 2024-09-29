#pragma once

#ifndef ButtonHandling_h
#define ButtonHandling_h
/////////////////////////////////////////////////////////////////
/* Watch for button presses on the two onboard buttons. When the left is pressed
 * record the current "sense" value as the low end of the reading range. 
 * Do the same for the right, but it's the high-end. 
 */

#include "Arduino.h"
#include <TFT_eSPI.h> // Hardware-specific library
#include "Button2.h"
#include "Display.h"

#define LEFT_BUTTON   0
#define RIGHT_BUTTON  35


void button_init();
void button_loop();

/////////////////////////////////////////////////////////////////
#endif
/////////////////////////////////////////////////////////////////
