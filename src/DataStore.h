#ifndef __TEMPERATURE_LOGGER_DATA_STORE_H__
#define __TEMPERATURE_LOGGER_DATA_STORE_H__

#include "config.h"
#include "Peripherals.h"
#include "PowerManager.h"

#define LOG_ENTRY_UNUSED 0xFF

#define LOG_PTR_CACHE_INVALID 0xFFFF
#define DATASTORE_RAW_OFFSET_TEMPERATURE 0
#define DATASTORE_RAW_OFFSET_HUMIDITY 1

class DataStore
{
private:
    static void advanceLogPtr();
    static uint16_t getLogPtr();
    static uint16_t logPtrCache;

public:
    static bool recordData();
    static uint8_t getStoredValue(uint16_t ix, uint8_t offset);
    static void wipeStoredData();
};

#endif