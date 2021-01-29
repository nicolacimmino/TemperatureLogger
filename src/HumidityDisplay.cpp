#include "HumidityDisplay.h"

void HumidityDisplay::setDefaultRange()
{
    this->minVal = 0;
    this->maxVal = 100;
}

float HumidityDisplay::getValueFromRawRecord(uint16_t record)
{
    return record & 0xFF;
}

float HumidityDisplay::getVTick()
{
    return 20;
}