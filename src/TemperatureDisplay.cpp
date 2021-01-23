#include "TemperatureDisplay.h"

TemperatureDisplay::TemperatureDisplay(Adafruit_SSD1306 *oled, uEEPROMLib *eeprom) : Display(oled)
{
    this->eeprom = eeprom;
}

void TemperatureDisplay::onBClick()
{
    this->plotAutoscale = !this->plotAutoscale;
}

void TemperatureDisplay::loop()
{
    if (PowerManager::level != PS_LEVEL_0)
    {
        return;
    }

    this->plotTemperature();
}

uint8_t TemperatureDisplay::temperatrureToYOffset(float temperature, int8_t minTemp, int8_t maxTemp)
{
    temperature = max(min(maxTemp, temperature), minTemp);
    return PLOT_Y_TOP + PLOT_Y_PIXELS - floor((PLOT_Y_PIXELS * (temperature - minTemp)) / (maxTemp - minTemp));
}

uint8_t TemperatureDisplay::plotIndexToXOffset(uint8_t ix)
{
    // This is simplified as the RAM shadow of the log is on purpose as
    // many points as the plot pixels.
    return ix + PLOT_X_LEFT;
}

void TemperatureDisplay::plotTemperature()
{
    if (!Status::replotNeeded)
    {
        return;
    }

    clearDisplay();

    this->oled->setCursor(0, 5);
    this->oled->print("BUSY....");
    this->oled->display();

    this->oled->drawLine(PLOT_X_LEFT, PLOT_Y_BOTTOM, PLOT_X_RIGHT, PLOT_Y_BOTTOM, SSD1306_WHITE);
    this->oled->drawLine(PLOT_X_LEFT, PLOT_Y_BOTTOM, PLOT_X_LEFT, PLOT_Y_TOP, SSD1306_WHITE);

    int8_t minTemp = 100;
    int8_t maxTemp = -100;

    if (this->plotAutoscale)
    {
        for (int ix = LOG_LENGTH_BYTES - 1; ix > 0; ix--)
        {
            int8_t temperature = (this->eeprom->eeprom_read(EEPROM_T_LOG_BASE + ix) - 127) / 2;
            minTemp = min(minTemp, temperature);
            maxTemp = max(maxTemp, temperature);
        }
    }
    else
    {
        minTemp = 15;
        maxTemp = 30;
    }

    uint8_t vTick = max(abs((maxTemp - minTemp) / 3), 2);

    for (uint8_t ix = PLOT_X_LEFT; ix < PLOT_X_RIGHT; ix += 4)
    {
        for (int8_t t = minTemp; t <= maxTemp; t += vTick)
        {
            this->oled->drawPixel(ix, temperatrureToYOffset(t, minTemp, maxTemp), SSD1306_WHITE);
            this->oled->setCursor(0, temperatrureToYOffset(t, minTemp, maxTemp) - (2 + (t == maxTemp ? -3 : 0)));
            this->oled->print(t);
        }
    }

    for (int ix = PLOT_X_LEFT; ix < PLOT_X_RIGHT; ix += PLOT_X_PIXELS / 12)
    {
        this->oled->drawLine(ix, PLOT_Y_BOTTOM - 1, ix, PLOT_Y_BOTTOM + 1, SSD1306_WHITE);
    }

    this->oled->display();

    float temperaturePointA = 0;

    uint16_t logPtr = this->eeprom->eeprom_read(EEPROM_LOG_PTR);

    bool overflow = false;

    for (int ix = LOG_LENGTH_BYTES - 1; ix > 0; ix--)
    {
        uint8_t readingPointer = (logPtr + ix) % LOG_LENGTH_BYTES;

        float temperaturePointB = (this->eeprom->eeprom_read(EEPROM_T_LOG_BASE + readingPointer) - 127) / 2;

        if (ix == LOG_LENGTH_BYTES - 1)
        {
            temperaturePointA = temperaturePointB;
            continue;
        }

        this->oled->drawLine(plotIndexToXOffset(ix + 1), temperatrureToYOffset(temperaturePointA, minTemp, maxTemp), plotIndexToXOffset(ix), temperatrureToYOffset(temperaturePointB, minTemp, maxTemp), SSD1306_WHITE);

        if (ix % 10 == 0)
        {
            this->oled->display();
        }

        if (temperaturePointA > maxTemp || temperaturePointA < minTemp)
        {
            overflow = true;
        }

        temperaturePointA = temperaturePointB;
    }

    this->oled->fillRect(0, 5, 50, 10, SSD1306_BLACK);

    if (overflow)
    {
        this->oled->setCursor(0, 5);
        this->oled->print("OVFL");
    }

    this->oled->display();

    Status::replotNeeded = false;
}
