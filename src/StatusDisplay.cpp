#include "StatusDisplay.h"

void StatusDisplay::onBClick()
{
    //
}

void StatusDisplay::loop()
{
    if (PowerManager::level != PS_LEVEL_0)
    {
        return;
    }

    this->displayStatus();
}

void StatusDisplay::displayStatus()
{
    this->clearDisplay();
    Peripherals::oled->setTextSize(1);

    Peripherals::oled->setCursor(0, 18);
    Peripherals::oled->print("VBAT:  ");
    Peripherals::oled->println(Status::getBatteryVoltage());
    Peripherals::oled->print("BAT:   ");
    Peripherals::oled->println(Status::getBatteryLevel());

    Peripherals::oled->display();    
}
