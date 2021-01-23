#include "TimeDisplay.h"

void TimeDisplay::onBClick()
{
    // Do nothing
}

void TimeDisplay::loop()
{
    if (PowerManager::level != PS_LEVEL_0)
    {
        return;
    }

    this->displayTime();
}

void TimeDisplay::displayTime()
{
    char text[16];
    char textB[6];

    Peripherals::rtc->refresh();

    this->clearDisplay();

    sprintf(text, "%02i:%02i", Peripherals::rtc->hour(), Peripherals::rtc->minute());
    Peripherals::oled->setTextSize(3);
    Peripherals::oled->setCursor(18, 21);
    Peripherals::oled->print(text);

    sprintf(text, "%02i", Peripherals::rtc->second());
    Peripherals::oled->setCursor(110, 35);
    Peripherals::oled->setTextSize(1);
    Peripherals::oled->print(text);

    float temperature = (SHT2x.GetTemperature() * 10) / 10.0;
    float humidity = round(SHT2x.GetHumidity());

    sprintf(text, "%sC", dtostrf(temperature, 3, 1, textB));
    Peripherals::oled->setTextSize(2);
    Peripherals::oled->setCursor(0, 50);
    Peripherals::oled->print(text);

    sprintf(text, "%s%%", dtostrf(humidity, 3, 0, textB));
    Peripherals::oled->setCursor(80, 50);
    Peripherals::oled->print(text);

    if (Status::isTimeSynced())
    {
        Peripherals::oled->drawBitmap(110, 22, timeSyncLogo, TIME_SYNC_LOGO_W, TIME_SYNC_LOGO_H, SSD1306_WHITE);
    }

    Peripherals::oled->display();
}