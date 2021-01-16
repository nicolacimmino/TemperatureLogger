#ifndef __TEMPERATURE_POWERMANAGER_H__
#define __TEMPERATURE_POWERMANAGER_H__

// L0   Fully operational (display ON, perhipherals on)
// L1   Background activity (display OFF, perhipherals on)
// L2   Low power (display OFF, perhipherals off)

#define PS_LEVEL_0 0
#define PS_LEVEL_1 1
#define PS_LEVEL_2 2

namespace PowerManager
{

unsigned long lastUserInteractionTime = millis();
uint8_t level = PS_LEVEL_0;
uint8_t previousLevel = PS_LEVEL_0;

void loop()
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

void onUserInteratcion()
{
    lastUserInteractionTime = millis();

    if(level != PS_LEVEL_0) {
        enterL0();
    }
}

void enterL0()
{
    previousLevel = level;
    level = PS_LEVEL_0;
    oled->ssd1306_command(SSD1306_DISPLAYON);
}

void enterL1()
{
    previousLevel = level;
    level = PS_LEVEL_1;
    //
}

void enterL2()
{
    previousLevel = level;
    level = PS_LEVEL_2;
    //
}

void restoreLevel()
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