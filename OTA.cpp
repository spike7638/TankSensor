#include "OTA.h"
#include "Persistence.h"

/////////////////////////////////////////////////////////////////
// OTA programming setup
/////////////////////////////////////////////////////////////////
/*
By this time, the Wifi connection shou;d be established, and if not, this code will fail. 
If it IS established, then OTA programming will work. 
*/


void OTAInit() 
{
   while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

  ArduinoOTA.begin();
  Serial.println("OTA Ready");
  Serial.print("OTA IP address: ");
  Serial.println(WiFi.localIP());
}

/////////////////////////////////////////////////////////////////