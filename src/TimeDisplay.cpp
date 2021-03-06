#include "TimeDisplay.h"

void TimeDisplay::onBClick()
{
    this->mode = (this->mode + 1) % TIME_MODES_COUNT;
    this->lastModeChangeTime = millis();
}

void TimeDisplay::onBLongPress()
{
    Status::timeSync();
}

void TimeDisplay::loop()
{
    if (PowerManager::level != PS_LEVEL_0 || Status::shouldAbortLoop())
    {
        return;
    }

    if (millis() - this->lastModeChangeTime > TIME_REVERT_TO_DEFAULT_DELAY_MS)
    {
        this->mode = TIME_MODE_TIME;
    }

    this->displayTime();
}

void TimeDisplay::displayTime()
{
    char text[16];
    char textB[6];

    Peripherals::rtc->refresh();

    this->clearDisplay();

    if (this->mode == TIME_MODE_TIME)
    {
        sprintf(text, "%02i%s%02i", Peripherals::rtc->hour(), (Peripherals::rtc->second() % 2 ? ":" : " "), Peripherals::rtc->minute());
        Peripherals::oled->setTextSize(3);
        Peripherals::oled->setCursor(18, 21);
        Peripherals::oled->print(text);

        sprintf(text, "%02i", Peripherals::rtc->second());
        Peripherals::oled->setCursor(110, 35);
        Peripherals::oled->setTextSize(1);
        Peripherals::oled->print(text);

        if (Status::isTimeSynced())
        {
            Peripherals::oled->drawBitmap(110, 22, timeSyncLogo, TIME_SYNC_LOGO_W, TIME_SYNC_LOGO_H, SSD1306_WHITE);
        }
    }
    else if (this->mode == TIME_MODE_DATE)
    {
        sprintf(text, "%02i-%02i-20%02i", Peripherals::rtc->day(), Peripherals::rtc->month(), Peripherals::rtc->year());
        Peripherals::oled->setTextSize(2);
        Peripherals::oled->setCursor(5, 25);
        Peripherals::oled->print(text);
    }

    sprintf(text, "%i.%iC", DataStore::getTemperatureDegrees(), DataStore::getTemperatureDecimalDegrees());
    Peripherals::oled->setTextSize(2);
    Peripherals::oled->setCursor(0, 50);
    Peripherals::oled->print(text);

    sprintf(text, "%i%%", DataStore::humidity);
    Peripherals::oled->setCursor(90, 50);
    Peripherals::oled->print(text);

    Peripherals::oled->display();
}