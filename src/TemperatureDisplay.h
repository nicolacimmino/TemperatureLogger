
#ifndef __TEMPERATURE_LOGGER_TEMP_DISPLAY_H__
#define __TEMPERATURE_LOGGER_TEMP_DISPLAY_H__

#include "Display.h"

class TemperatureDisplay : public Display
{
private:
    uEEPROMLib *eeprom;
    bool plotAutoscale = false;

    uint8_t temperatrureToYOffset(float temperature, int8_t minTemp, int8_t maxTemp);
    uint8_t plotIndexToXOffset(uint8_t ix);
    void plotTemperature();

public:    
    void loop();
    void onBClick();
};

#endif