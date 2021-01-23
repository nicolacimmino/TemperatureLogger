#include "Display.h"

Display::Display(Adafruit_SSD1306 *oled)
{
    this->oled = oled;
}

void Display::clearDisplay()
{
    this->oled->clearDisplay();
    this->oled->setTextSize(1);
    this->oled->setTextColor(SSD1306_WHITE);
    this->plotBatterLevel();
}

void Display::plotBatterLevel()
{
    uint8_t batteryLevel = Status::getBatteryLevel();
    this->oled->setCursor(80, 5);
    this->oled->print(batteryLevel);
    this->oled->print("%");

    this->oled->drawBitmap(110, 5, batteryLogo[round(batteryLevel / 25.0)], BATTERY_LOGO_W, BATTERY_LOGO_H, SSD1306_WHITE);
}
