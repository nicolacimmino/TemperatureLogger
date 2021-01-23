
#include "PowerManager.h"

unsigned long PowerManager::lastUserInteractionTime = millis();
uint8_t PowerManager::level = PS_LEVEL_0;
uint8_t PowerManager::previousLevel = PS_LEVEL_0;

void PowerManager::enterL0()
{
    previousLevel = level;

    digitalWrite(PIN_PWR_AUX_DEVS, HIGH);
    delay(PS_BUS_GUARD_TIME_MS);
    Peripherals::oled->ssd1306_command(SSD1306_DISPLAYON);

    level = PS_LEVEL_0;
}

void PowerManager::enterL1()
{
    previousLevel = level;

    digitalWrite(PIN_PWR_AUX_DEVS, HIGH);
    delay(PS_BUS_GUARD_TIME_MS);

    level = PS_LEVEL_1;
    //
}

void PowerManager::enterL2()
{
    previousLevel = level;

    Peripherals::oled->ssd1306_command(SSD1306_DISPLAYOFF);
    digitalWrite(PIN_PWR_AUX_DEVS, LOW);

    level = PS_LEVEL_2;
    //
}

void PowerManager::loop()
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

void PowerManager::onUserInteratcion()
{
    lastUserInteractionTime = millis();

    if (level != PS_LEVEL_0)
    {
        enterL0();
    }
}

void PowerManager::restoreLevel()
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
