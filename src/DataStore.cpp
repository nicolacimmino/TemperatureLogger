#include "DataStore.h"
#include "ModeManager.h"

uint16_t DataStore::logPtrCache = LOG_PTR_CACHE_INVALID;
unsigned long DataStore::lastRecordTime = 0;
unsigned long DataStore::lastSampleTime = 0;
float DataStore::temperature = DATA_STORE_VALUE_NOT_INITIALIZED;
uint8_t DataStore::humidity = DATA_STORE_VALUE_NOT_INITIALIZED;

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

void DataStore::loop()
{
    if (DataStore::lastSampleTime != 0 && millis() - DataStore::lastSampleTime < SAMPLE_DATA_INTERVAL_MS)
    {
        return;
    }
    DataStore::lastSampleTime = millis();

    // TODO: move to power manager
    bool restoreLevel = false;
    if (PowerManager::level > PS_LEVEL_1)
    {
        PowerManager::enterL1();
        restoreLevel = true;
    }

    if (DataStore::temperature == DATA_STORE_VALUE_NOT_INITIALIZED)
    {
        DataStore::temperature = SHT2x.GetTemperature();
        DataStore::humidity = round(SHT2x.GetHumidity());
    }
    else
    {

        DataStore::temperature = (DataStore::temperature * 0.9) + (SHT2x.GetTemperature() * 0.1);
        DataStore::humidity = round((DataStore::humidity * 0.9) + (SHT2x.GetHumidity() * 0.1));
    }

    if (millis() - DataStore::lastRecordTime > RECORD_DATA_INTERVAL_MS)
    {
        DataStore::lastRecordTime = millis();
        DataStore::recordData();
    }

    if (restoreLevel)
    {
        PowerManager::restoreLevel();
    }
}

void DataStore::recordData()
{
    uint8_t temperatureEncoded = 127 + (DataStore::temperature * 2);

    uint16_t logPtr = DataStore::getLogPtr();
    Peripherals::eeprom->eeprom_write(EEPROM_LOG_BASE + (logPtr * LOG_ENTRY_BYTES), temperatureEncoded);
    Peripherals::eeprom->eeprom_write(EEPROM_LOG_BASE + (logPtr * LOG_ENTRY_BYTES) + 1, DataStore::humidity);

    DataStore::advanceLogPtr();

    ModeManager::currentDisplay->onDataStoreChange();
}

int8_t DataStore::getTemperatureDegrees()
{    
    return floor(DataStore::temperature);    
}

uint8_t DataStore::getTemperatureDecimalDegrees()
{
    return 10 * (DataStore::temperature - floor(DataStore::temperature));
}