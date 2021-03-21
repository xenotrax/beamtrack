#include <OneButton.h>
#include "power.h"

OneButton btn = OneButton(GPIO_NUM_38, true, true);

static void buttonClick()
{
    Serial.println("\nButton pressed, going to sleep.");
    powerSleep();
}

void buttonSetup()
{
    btn.attachClick(buttonClick);
}

void buttonLoop()
{
    btn.tick();
}