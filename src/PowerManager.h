#ifndef __TEMPERATURE_POWERMANAGER_H__
#define __TEMPERATURE_POWERMANAGER_H__

#include <Arduino.h>
#include <Adafruit_SSD1306.h>
#include "config.h"

// L0   Fully operational (display ON, perhipherals on)
// L1   Background activity (display OFF, perhipherals on)
// L2   Low power (display OFF, perhipherals off)

#define PS_LEVEL_0 0
#define PS_LEVEL_1 1
#define PS_LEVEL_2 2

// Some I2C devices can interfere with the bus
// for a short time during power up. This is a
// guard between powring back up aux devs and
// accessing the bus.
#define PS_BUS_GUARD_TIME_MS 100

class PowerManager
{
private:
    static unsigned long lastUserInteractionTime;
    static uint8_t previousLevel;

public:
    static uint8_t level;
    static Adafruit_SSD1306 *oled;

    static void enterL0();
    static void enterL1();
    static void enterL2();
    static void loop();
    static void onUserInteratcion();
    static void restoreLevel();
};

#endif