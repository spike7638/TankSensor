#include "hal/adc_types.h"
#include "TouchAndSense.h"

/////////////////////////////////////////////////////////////////
// Touch and Sense connections
/////////////////////////////////////////////////////////////////

void touchAndSenseInit() 
{
  pinMode (ledPin, OUTPUT); // debugging LED

}

void showInputState(int inputValue)
{
  if(inputValue < DEBUG_THRESHOLD){
    digitalWrite(ledPin, HIGH);
  }
  else{
    digitalWrite(ledPin, LOW);
  }
}

int getSenseValue()
{
  static int senseValue;
  senseValue = analogRead(sensePin);
  //showInputState(senseValue); // uncomment to debug whether a sensor is giving a reasonable value.
  //Serial.println("Sensor: " + String(senseValue));
  return senseValue;
}

bool getTouchState()
{
  return (touchRead(touchPin) < TOUCH_THRESHOLD);
}

