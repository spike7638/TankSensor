#include "HardwareSerial.h"
#include "Persistence.h"
#include "TFT_eSPI.h"
/////////////////////////////////////////////////////////////////
/*
  Display.cpp 
*/
/////////////////////////////////////////////////////////////////

#include "Arduino.h"
#include "Display.h"

// Add a definition of the backlight pin, oddly missing from the TFT thing
#define TFT_BL 4
static int BGColor;
static TFT_eSPI tft = TFT_eSPI(); 
static TFT_eSprite img = TFT_eSprite(&tft);
static int16_t xpos = tft.width() / 2;
static int16_t ypos = tft.height() / 2;

#define TEXT_FONT &FreeSansBold12pt7b

/////////////////////////////////////////////////////////////////

/* Display some text, in a larger font, in the upper left corner of the screeen */
void displayMessage(String s, float secs)
{
  //tft.setTextSize(1);           // No size multiplier
  tft.setFreeFont(TEXT_FONT);
  tft.setRotation(1);           // change rotation to have more room
  tft.fillScreen(TFT_BLACK);    // Fill the screen with black colour
  tft.setTextColor(TFT_GREEN, TFT_BLACK);   // Set text color to green and padding to black
  tft.drawString(s, 1, 1);  
  tft.setRotation(2);           // and change rotation back again
  delay(int(1000 * secs));
}

/* Display a string, in a boring font, mid-display */
void displayText(String s)
{
  tft.setTextSize(1);           // No size multiplier
  //tft.setFreeFont(TEXT_FONT);
  tft.setRotation(1);           // change rotation to have more room
  tft.fillScreen(TFT_BLACK);    // Fill the screen with black colour
  tft.setTextColor(TFT_GREEN, TFT_BLACK);   // Set text color to green and padding to black
  tft.drawCentreString(s, ypos, 50, 4);  
  tft.setRotation(2);           // and change rotation back again
}

void displayInit() {
  tft.begin();
  tft.setRotation(2);
  tft.fillScreen(TFT_NAVY);
  img.createSprite(tft.width(), tft.height());

  pinMode(TFT_BL, OUTPUT); // Make backlight pin controllable
  digitalWrite(TFT_BL, LOW); // Should force backlight on
  tft.writecommand(ST7789_DISPON);// Switch on the display
  tft.writecommand(ST7789_SLPIN);// Awaken the display driver
  //tft.fillScreen(TFT_YELLOW);
  //delay(1000);
  //tft.fillScreen(TFT_NAVY);
}

void displaySetBackground(int color){
  BGColor = color;
}

/* Display a bar graph showing the range of possible values as an unfilled rectangle
** with a filled region up to the "value" (in a range between lowerLim and upperLim),
** and a red line drawn at the redLine value to indicate a critical reading. 
** As an example, if lowerLim = 30 and upperLim = 130, and the value is 55, then 
** the bar graph will be 25% filled, because 55 is one quarter of the way from 30
** to 130. If redLine is 105, then there'll be a red line at the 3/4 height on the bar
** graph.
*/ 
void displayShowLevel(int value, int lowerLim, int upperLim, int redLine)
{
  static int16_t xpos = tft.width() / 2;
  static int32_t buffer = 10; // pixels
  static int32_t boxLineWidth = 3; // pixels
  static int32_t redLineWidth = 4; // pixels
  static int32_t boxWidth = tft.width()/3;
  static int32_t boxHeight = tft.height() - 2 * buffer; 

  int barHeight = ((boxHeight - 2 * boxLineWidth) * (value - lowerLim))/(upperLim - lowerLim);
  //Serial.print("barHeight: ");
  //Serial.println(barHeight);
  img.fillScreen(BGColor);

  img.fillRect(xpos - boxWidth/2, buffer,  boxWidth, boxHeight, TFT_YELLOW);
  img.fillRect(xpos - boxWidth/2 + boxLineWidth, buffer + boxLineWidth,  boxWidth-2*boxLineWidth, boxHeight - 2 * boxLineWidth, TFT_NAVY);

  img.fillRect(xpos - boxWidth/2 + boxLineWidth, buffer + boxLineWidth,  boxWidth-2*boxLineWidth, barHeight, TFT_YELLOW);
  int redLineHeight = ((boxHeight - 2 * boxLineWidth) * (redLine - lowerLim))/(upperLim - lowerLim);
  img.fillRect(xpos - boxWidth/2 + boxLineWidth, buffer + (redLineHeight - redLineWidth/2),  boxWidth-2*boxLineWidth, redLineWidth, TFT_RED);
  img.pushSprite(0,0);
}

void displayActivate(bool b)
{
    if (b) {
    digitalWrite(TFT_BL, HIGH); // Should force backlight on
    tft.writecommand(ST7789_DISPON);// Switch on the display
    tft.writecommand(ST7789_SLPOUT);// Awaken the display driver
  }
  else {
      digitalWrite(TFT_BL, LOW); // Should force backlight off
      tft.writecommand(ST7789_DISPOFF);// Switch off the display
      tft.writecommand(ST7789_SLPIN);// Sleep the display driver
    }
}
/////////////////////////////////////////////////////////////////