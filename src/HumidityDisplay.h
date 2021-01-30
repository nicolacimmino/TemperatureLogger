
#ifndef __TEMPERATURE_LOGGER_HUMIDITY_DISPLAY_H__
#define __TEMPERATURE_LOGGER_HUMIDITY_DISPLAY_H__

#include "GraphDisplay.h"

class HumidityDisplay : public GraphDisplay
{
private:
protected:
    void setDefaultRange();
    float getValueFromRawRecord(uint8_t record);
    uint8_t getRawRecordOffset();
    float getVTick();

public:
};

#endif