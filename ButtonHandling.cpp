#include "Persistence.h"
#include "Arduino.h"
#include "ButtonHandling.h"
#include "TouchAndSense.h"

static Button2 btn1(LEFT_BUTTON);
static Button2 btn2(RIGHT_BUTTON);

void button_init()
{
    btn1.setPressedHandler([](Button2 & b) {
        setLowerLimit(getSenseValue());
        displaySetBackground(TFT_GREEN);
    });

    btn2.setPressedHandler([](Button2 & b) {
        setUpperLimit(getSenseValue());
        displaySetBackground(TFT_WHITE);
    });
}


void button_loop()
{
    btn1.loop();
    btn2.loop();
}