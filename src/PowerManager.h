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
public:
static unsigned long lastUserInteractionTime;
static uint8_t level;
static uint8_t previousLevel;
static Adafruit_SSD1306 *oled;

static void enterL0()
{
    previousLevel = level;

    digitalWrite(PIN_PWR_AUX_DEVS, HIGH);
    delay(PS_BUS_GUARD_TIME_MS);
    PowerManager::oled->ssd1306_command(SSD1306_DISPLAYON);

    level = PS_LEVEL_0;
}

static void enterL1()
{
    previousLevel = level;

    digitalWrite(PIN_PWR_AUX_DEVS, HIGH);
    delay(PS_BUS_GUARD_TIME_MS);

    level = PS_LEVEL_1;
    //
}

static void enterL2()
{
    previousLevel = level;

    PowerManager::oled->ssd1306_command(SSD1306_DISPLAYOFF);
    digitalWrite(PIN_PWR_AUX_DEVS, LOW);

    level = PS_LEVEL_2;
    //
}

static void loop()
{
    if (level > PS_LEVEL_0)
    {
        return;
    }

    if (millis() - lastUserInteractionTime > POWER_SAVE_TIMEOUT_MS)
    {
        enterL2();
    }
}

static void onUserInteratcion()
{
    lastUserInteractionTime = millis();

    if (level != PS_LEVEL_0)
    {
        enterL0();
    }
}

static void restoreLevel()
{
    if (previousLevel != level)
    {
        switch (previousLevel)
        {
        case (PS_LEVEL_0):
            enterL0();
            break;
        case (PS_LEVEL_1):
            enterL1();
            break;
        case (PS_LEVEL_2):
            enterL2();
            break;
        }
    }
}

}; // namespace PowerManager

#endif