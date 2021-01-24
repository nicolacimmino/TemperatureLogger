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
    Peripherals::oled->print(F("VBAT:  "));
    Peripherals::oled->print(Status::getBatteryVoltage());
    Peripherals::oled->println(F(" mV"));
    Peripherals::oled->print(F("BAT:   "));
    Peripherals::oled->print(Status::getBatteryLevel());
    Peripherals::oled->println(F(" %"));
    Peripherals::oled->print(F("RAM:   "));
    Peripherals::oled->print(Status::getFreeRamBytes());
    Peripherals::oled->println(F(" Bytes"));
    
    Peripherals::oled->display();    
}
