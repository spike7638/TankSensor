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

void drawGauge(int xLoc, int yLoc, int16_t value, int16_t criticalValue, int16_t criticalDirection);
int percentToAngle(int percent);
void drawSegment(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint32_t fg_color, uint32_t bg_color );
void createSprites();


///////////////////////////////////////////////////////////////// Forward def'ns

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

  createSprites(); //used for "fuel gauge" display
}

void displaySetBackground(int color){
  BGColor = color;
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

/* Display a bar graph showing the range of possible values as an unfilled rectangle
** with a filled region up to the "value" (in a range between lowerLim and upperLim),
** and a red line drawn at the redLine value to indicate a critical reading. 
** As an example, if lowerLim = 30 and upperLim = 130, and the value is 55, then 
** the bar graph will be 25% filled, because 55 is one quarter of the way from 30
** to 130. If redLine is 105, then there'll be a red line at the 3/4 height on the bar
** graph.
*/ 
void displayShowLevelBar(int value, int lowerLim, int upperLim, int redLine)
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

/* ============ Gauge Display ======*/
/* Everything from here down supports the gauge-display instead of the bar-graph */
#define DEG2RAD 0.0174532925

const int displayWidth = tft.width();
const int displayHeight = tft.height();

const int gaugeRadius = 105;
const int pointerLength = 0.75*gaugeRadius;
const int lineThickness = 8; // pixels
const int angleMin = 34;
const int angleMax = 146;
const int majorTickCount = 3; 
const int minorTickCount = 5;
const uint32_t pointerColor = TFT_YELLOW;
const uint32_t meterColor = TFT_BLACK;
const uint32_t dialColor = 0x6b6d; // greyish
const uint32_t alertColor = TFT_RED;


const int majorTickLength = gaugeRadius/6;
const int minorTickLength = majorTickLength/2;
const int majorTickGap = (angleMax - angleMin)/ (majorTickCount-1);
const int minorTickGap = (angleMax - angleMin)/ (minorTickCount-1);


const int xLoc = 4*displayWidth / 4;
const int yLoc = -6 + displayHeight / 2;

TFT_eSprite needle = TFT_eSprite(&tft);  // Create Sprite object for the needle in red
TFT_eSprite canvas = TFT_eSprite(&tft); // Sprite for drawing


void displayShowLevelGauge(int value, int lowerLim, int upperLim, int redLine, int direction)
{
  int16_t percent = round((100.0 * (value - lowerLim)) / float(upperLim - lowerLim));
  int16_t redLinePercent = round((100.0 * (redLine - lowerLim))/float(upperLim - lowerLim));
  drawGauge(xLoc, yLoc, percent, redLinePercent, direction); 
}

int percentToAngle(int percent){
  return 0.0 + angleMin + (percent * (angleMax - angleMin))/ 100.0;
}

void drawGauge(int xLoc, int yLoc, int16_t value, int16_t criticalValue, int16_t criticalDirection){
// draw background, 
// draw red arc for critical area
// draw needle
// copy to display
  uint16_t x0, y0, x1, y1;
  canvas.fillScreen(dialColor);
  
  // XXX Fix next two lines to deal with critical range
  canvas.drawSmoothArc(xLoc, yLoc, gaugeRadius + lineThickness, gaugeRadius, angleMin, angleMax, meterColor, dialColor, true);

  int angle1 = percentToAngle(criticalValue);
  int angle2 = (criticalDirection == +1) ? angleMax : angleMin;

  canvas.drawSmoothArc(xLoc, yLoc, gaugeRadius + lineThickness, gaugeRadius, min(angle1, angle2), max(angle1, angle2), alertColor, dialColor, true);
 

  for (int i = angleMin; i <= angleMax; i+=majorTickGap){
    float c = cos((i+90) * DEG2RAD);
    float s = sin((i+90) * DEG2RAD);
    x0 = c * (gaugeRadius) + xLoc ;
    y0 = s * (gaugeRadius) + yLoc;
    x1 = c * (gaugeRadius - majorTickLength) + xLoc;
    y1 = s * (gaugeRadius - majorTickLength) + yLoc;

    int angle = percentToAngle(value);

    drawSegment(x0, y0, x1, y1, meterColor, dialColor);
    needle.pushRotated(&canvas, angle, TFT_TRANSPARENT);
    //canvas.pushSprite(xLoc, yLoc);

}


  
  for (int i = angleMin; i <= angleMax; i+=minorTickGap){
    float c = cos((i+90) * DEG2RAD);
    float s = sin((i+90) * DEG2RAD);
    x0 = c * (gaugeRadius) + xLoc ;
    y0 = s * (gaugeRadius) + yLoc;
    x1 = c * (gaugeRadius - minorTickLength) + xLoc;
    y1 = s * (gaugeRadius - minorTickLength) + yLoc;
    drawSegment(x0, y0, x1, y1, meterColor, dialColor);
  }
  
  canvas.pushSprite(0, 0);// testing
}

void drawSegment(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint32_t fg_color, uint32_t bg_color )
{
  float dx = x1 - x0; 
  float dy = y1 - y0;
  float len = sqrt(dx * dx + dy * dy);
  float iLen = round(len);

  for (int i = 0; i <= iLen; i++){
    float x = round(x0 + i * dx/len); 
    float y = round(y0 + i * dy/len);
    canvas.drawSmoothCircle(x, y, lineThickness/2, fg_color, bg_color);
  }
}

void createSprites(){
  canvas.createSprite(1.25 * gaugeRadius, 2*gaugeRadius);
  canvas.setPivot(xLoc, yLoc);
  canvas.fillSprite(TFT_TRANSPARENT);

  needle.createSprite(21, 80);
  needle.fillSprite(TFT_TRANSPARENT);
  needle.fillTriangle(6, 10, 14, 10, 10, 4 + pointerLength, pointerColor);
  needle.fillCircle(10, 10, 8, meterColor);
  needle.setPivot(10, 0);


}




/////////////////////////////////////////////////////////////////