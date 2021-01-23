#include "Display.h"

void Display::onDisplayInvalidated()
{
    this->replotNeeded = true;
}

void Display::clearDisplay()
{
    Peripherals::oled->clearDisplay();
    Peripherals::oled->setTextSize(1);
    Peripherals::oled->setTextColor(SSD1306_WHITE);
    this->plotBatterLevel();
}

void Display::plotBatterLevel()
{
    uint8_t batteryLevel = Status::getBatteryLevel();
    Peripherals::oled->setCursor(80, 5);
    Peripherals::oled->print(batteryLevel);
    Peripherals::oled->print("%");

    Peripherals::oled->drawBitmap(110, 5, batteryLogo[round(batteryLevel / 25.0)], BATTERY_LOGO_W, BATTERY_LOGO_H, SSD1306_WHITE);
}
