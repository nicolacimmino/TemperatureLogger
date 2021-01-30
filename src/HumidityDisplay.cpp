#include "HumidityDisplay.h"

void HumidityDisplay::setDefaultRange()
{
    this->minVal = 0;
    this->maxVal = 100;
}

uint8_t HumidityDisplay::getRawRecordOffset()
{
    return DATASTORE_RAW_OFFSET_HUMIDITY;
}

float HumidityDisplay::getValueFromRawRecord(uint8_t record)
{
    return record;
}

float HumidityDisplay::getVTick()
{
    return 25;
}