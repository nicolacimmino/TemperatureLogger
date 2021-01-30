#include "DataStore.h"

uint16_t DataStore::logPtrCache = LOG_PTR_CACHE_INVALID;

uint16_t DataStore::getLogPtr()
{
    if (DataStore::logPtrCache == LOG_PTR_CACHE_INVALID)
    {
        DataStore::logPtrCache = Peripherals::eeprom->eeprom_read(EEPROM_LOG_PTR);
    }

    return DataStore::logPtrCache;
}

void DataStore::advanceLogPtr()
{
    Peripherals::eeprom->eeprom_write(EEPROM_LOG_PTR, (DataStore::getLogPtr() + 1) % LOG_LENGTH_POINTS);
    DataStore::logPtrCache = LOG_PTR_CACHE_INVALID;
}

uint16_t DataStore::getStoredValue(uint16_t ix)
{
    uint8_t readingPointer = (DataStore::getLogPtr() + ix) % LOG_LENGTH_POINTS;

    return Peripherals::eeprom->eeprom_read(EEPROM_T_LOG_BASE + (readingPointer * LOG_ENTRY_BYTES)) << 8 | Peripherals::eeprom->eeprom_read(EEPROM_T_LOG_BASE + (readingPointer * LOG_ENTRY_BYTES) + 1);
}

bool DataStore::recordData()
{
    static unsigned long lastRecord = millis();
    if (millis() - lastRecord < RECORD_DATA_INTERVAL_MS)
    {
        return false;
    }
    lastRecord = millis();

    bool restoreLevel = false;
    if (PowerManager::level > PS_LEVEL_1)
    {
        PowerManager::enterL1();
        restoreLevel = true;
    }

    uint8_t temperatureEncoded = 127 + (SHT2x.GetTemperature() * 2);
    uint8_t humidityEncoded = SHT2x.GetHumidity();

    uint16_t logPtr = DataStore::getLogPtr();
    Peripherals::eeprom->eeprom_write(EEPROM_T_LOG_BASE + (logPtr * LOG_ENTRY_BYTES), temperatureEncoded);
    Peripherals::eeprom->eeprom_write(EEPROM_T_LOG_BASE + (logPtr * LOG_ENTRY_BYTES) + 1, humidityEncoded);

    DataStore::advanceLogPtr();

    if (restoreLevel)
    {
        PowerManager::restoreLevel();
    }

    return true;
}