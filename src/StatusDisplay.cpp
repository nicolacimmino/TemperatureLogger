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
    Peripherals::oled->print(Status::getBatteryVoltage());
    Peripherals::oled->println(" mV");
    Peripherals::oled->print("BAT:   ");
    Peripherals::oled->print(Status::getBatteryLevel());
    Peripherals::oled->println(" %");
    Peripherals::oled->print("RAM:   ");
    Peripherals::oled->print(Status::getFreeRamBytes());
    Peripherals::oled->println(" Bytes");
    
    Peripherals::oled->display();    
}
