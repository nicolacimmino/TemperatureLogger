#ifndef __TEMPERATURE_LOGGER_DATA_STORE_H__
#define __TEMPERATURE_LOGGER_DATA_STORE_H__

#include "config.h"
#include "Peripherals.h"
#include "PowerManager.h"


#define LOG_ENTRY_UNUSED 0xFF

#define LOG_PTR_CACHE_INVALID 0xFFFF
#define DATA_STORE_VALUE_NOT_INITIALIZED 0xFF
#define DATASTORE_RAW_OFFSET_TEMPERATURE 0
#define DATASTORE_RAW_OFFSET_HUMIDITY 1

class DataStore
{
private:
    static unsigned long lastRecordTime;
    static unsigned long lastSampleTime;
    static void advanceLogPtr();
    static uint16_t getLogPtr();
    static uint16_t logPtrCache;
    static void recordData();
    static float temperature;
public:
    static int8_t getTemperatureDegrees();
    static uint8_t getTemperatureDecimalDegrees();
    static uint8_t humidity;
    static void loop();
    static uint8_t getStoredValue(uint16_t ix, uint8_t offset);
    static void wipeStoredData();
};

#endif