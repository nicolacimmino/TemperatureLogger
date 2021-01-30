#include "TemperatureDisplay.h"

void TemperatureDisplay::setDefaultRange()
{
    this->minVal = 15;
    this->maxVal = 30;
}

uint8_t TemperatureDisplay::getRawRecordOffset()
{
    return DATASTORE_RAW_OFFSET_TEMPERATURE;
}

float TemperatureDisplay::getValueFromRawRecord(uint8_t record)
{
    return (record - 127) / 2;
}

float TemperatureDisplay::getVTick()
{
    return max(abs((this->maxVal - this->minVal) / 3), 2);
}

void TemperatureDisplay::printPlotTitle()
{    
    Peripherals::oled->print(F("Temperature"));
}