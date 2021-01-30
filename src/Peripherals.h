#ifndef __TEMPERATURE_PERIPHERALS_H__
#define __TEMPERATURE_PERIPHERALS_H__

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Sodaq_SHT2x.h>
#include <uRTCLib.h>
#include <uEEPROMLib.h>
#include <DCServicesLite.h>
#include "config.h"
#include "Button.h"

class Peripherals
{

public:
    static uRTCLib *rtc;
    static Adafruit_SSD1306 *oled;
    static uEEPROMLib *eeprom;
    static Button *buttonA;
    static Button *buttonB;
    static DCServicesLite *dcServices;
    static void setup();
};

#endif