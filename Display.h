#pragma once

#ifndef Display_h
#define Display_h

#include "Arduino.h"
#include <TFT_eSPI.h> // Hardware-specific library

/////////////////////////////////////////////////////////////////
void displayInit();
void displaySetBackground(int color);
void displayShowLevelBar(int value, int lowerLim, int upperLim, int redLine);
void displayMessage(String s, float secs);
void displayText(String s); // show this string in a large font
void displayActivate(bool b);
void displayShowLevelGauge(int value, int lowerLim, int upperLim, int redLine, int direction);

/////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////
#endif
/////////////////////////////////////////////////////////////////

