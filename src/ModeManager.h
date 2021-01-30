#ifndef __TEMPERATURE_MODE_MANAGER_H__
#define __TEMPERATURE_MODE_MANAGER_H__

#include <Arduino.h>
#include "config.h"
#include "Display.h"
#include "TimeDisplay.h"
#include "TemperatureDisplay.h"
#include "HumidityDisplay.h"
#include "StatusDisplay.h"


class ModeManager
{
private:    
    static uint8_t mode;
    static void enterMode();

public:
    static Display *currentDisplay;
    static void setup();    
    static void changeMode();
};

#endif