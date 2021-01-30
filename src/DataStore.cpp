#include "DataStore.h"

uint16_t DataStore::logPtrCache = LOG_PTR_CACHE_INVALID;

void DataStore::wipeStoredData()
{
    Peripherals::oled->setTextSize(2);
    Peripherals::oled->setTextColor(SSD1306_WHITE);
    
    for (uint16_t ix = 0; ix < (LOG_LENGTH_POINTS * LOG_ENTRY_BYTES); ix++)
    {
        Peripherals::eeprom->eeprom_write(EEPROM_LOG_BASE + ix, LOG_ENTRY_UNUSED);

        if (ix % 10 == 0)
        {
            Peripherals::oled->clearDisplay();
            Peripherals::oled->setCursor(0, 20);
            Peripherals::oled->print("Wiping ");
            Peripherals::oled->print(ix * 100 / (LOG_LENGTH_POINTS * LOG_ENTRY_BYTES));
            Peripherals::oled->print("%");
            Peripherals::oled->display();
        }
    }

    Peripherals::eeprom->eeprom_write(EEPROM_LOG_PTR, 0);
}

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

uint8_t DataStore::getStoredValue(uint16_t ix, uint8_t offset)
{
    uint8_t readingPointer = (DataStore::getLogPtr() + ix) % LOG_LENGTH_POINTS;
    readingPointer = EEPROM_LOG_BASE + (readingPointer * LOG_ENTRY_BYTES) + offset;

    return Peripherals::eeprom->eeprom_read(readingPointer);
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
    Peripherals::eeprom->eeprom_write(EEPROM_LOG_BASE + (logPtr * LOG_ENTRY_BYTES), temperatureEncoded);
    Peripherals::eeprom->eeprom_write(EEPROM_LOG_BASE + (logPtr * LOG_ENTRY_BYTES) + 1, humidityEncoded);

    DataStore::advanceLogPtr();

    if (restoreLevel)
    {
        PowerManager::restoreLevel();
    }

    return true;
}