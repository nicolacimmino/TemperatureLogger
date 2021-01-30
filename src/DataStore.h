#ifndef __TEMPERATURE_LOGGER_DATA_STORE_H__
#define __TEMPERATURE_LOGGER_DATA_STORE_H__

#include "config.h"
#include "Peripherals.h"
#include "PowerManager.h"

#define LOG_PTR_CACHE_INVALID 0xFFFF

class DataStore
{
private:
    static void advanceLogPtr();
    static uint16_t getLogPtr();
    static uint16_t logPtrCache;
public:
    static bool recordData();
    static uint16_t getStoredValue(uint16_t ix);
};

#endif