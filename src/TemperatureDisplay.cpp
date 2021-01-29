#include "TemperatureDisplay.h"

void TemperatureDisplay::setDefaultRange()
{
    this->minVal = 15;
    this->maxVal = 30;
}

float TemperatureDisplay::getValueFromRawRecord(uint16_t record)
{
    return ((record >> 8) - 127) / 2;
}

float TemperatureDisplay::getVTick()
{
    return max(abs((this->maxVal - this->minVal) / 3), 2);
}