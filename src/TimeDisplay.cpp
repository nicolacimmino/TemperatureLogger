#include "TimeDisplay.h"

TimeDisplay::TimeDisplay(Adafruit_SSD1306 *oled, uRTCLib *rtc) : Display(oled)
{
    this->rtc = rtc;
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

    this->rtc->refresh();

    this->clearDisplay();

    sprintf(text, "%02i:%02i", this->rtc->hour(), this->rtc->minute());
    this->oled->setTextSize(3);
    this->oled->setCursor(18, 21);
    this->oled->print(text);

    sprintf(text, "%02i", this->rtc->second());
    this->oled->setCursor(110, 35);
    this->oled->setTextSize(1);
    this->oled->print(text);

    float temperature = (SHT2x.GetTemperature() * 10) / 10.0;
    float humidity = round(SHT2x.GetHumidity());

    sprintf(text, "%sC", dtostrf(temperature, 3, 1, textB));
    this->oled->setTextSize(2);
    this->oled->setCursor(0, 50);
    this->oled->print(text);

    sprintf(text, "%s%%", dtostrf(humidity, 3, 0, textB));
    this->oled->setCursor(80, 50);
    this->oled->print(text);

    if (Status::isTimeSynced())
    {
        this->oled->drawBitmap(110, 22, timeSyncLogo, TIME_SYNC_LOGO_W, TIME_SYNC_LOGO_H, SSD1306_WHITE);
    }

    this->oled->display();
}