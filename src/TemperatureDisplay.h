
#ifndef __TEMPERATURE_LOGGER_TEMP_DISPLAY_H__
#define __TEMPERATURE_LOGGER_TEMP_DISPLAY_H__

#include "Display.h"

class TemperatureDisplay : public Display
{
private:
    uEEPROMLib *eeprom;
    bool plotAutoscale = false;
    uint8_t minTemp = 15;
    uint8_t maxTemp = 30;

    uint8_t temperatrureToYOffset(float temperature);
    uint8_t plotIndexToXOffset(uint8_t ix);
    void plotTemperature();

public:
    void loop();
    void onBClick();
};

#endif