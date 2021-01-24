#include "TemperatureDisplay.h"

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

uint8_t TemperatureDisplay::temperatrureToYOffset(float temperature)
{
    temperature = max(min(this->maxTemp, temperature), this->minTemp);
    return PLOT_Y_TOP + PLOT_Y_PIXELS - floor((PLOT_Y_PIXELS * (temperature - this->minTemp)) / (this->maxTemp - this->minTemp));
}

uint8_t TemperatureDisplay::plotIndexToXOffset(uint8_t ix)
{
    // This is simplified as the RAM shadow of the log is on purpose as
    // many points as the plot pixels.
    return ix + PLOT_X_LEFT;
}

void TemperatureDisplay::plotTemperature()
{
    if (!this->replotNeeded)
    {
        return;
    }

    this->clearDisplay();

    Peripherals::oled->setCursor(0, 5);
    Peripherals::oled->print("BUSY....");
    Peripherals::oled->display();

    Peripherals::oled->drawLine(PLOT_X_LEFT, PLOT_Y_BOTTOM, PLOT_X_RIGHT, PLOT_Y_BOTTOM, SSD1306_WHITE);
    Peripherals::oled->drawLine(PLOT_X_LEFT, PLOT_Y_BOTTOM, PLOT_X_LEFT, PLOT_Y_TOP, SSD1306_WHITE);

    this->minTemp = 15;
    this->maxTemp = 30;
    if (this->plotAutoscale)
    {
        this->minTemp = 100;
        this->maxTemp = -100;
        for (int ix = LOG_LENGTH_BYTES - 1; ix > 0; ix--)
        {
            if (Peripherals::buttonA->isPressed() || Peripherals::buttonB->isPressed())
            {
                Status::abortLoop = true;
                return;
            }

            int8_t temperature = (Peripherals::eeprom->eeprom_read(EEPROM_T_LOG_BASE + ix) - 127) / 2;
            this->minTemp = min(this->minTemp, temperature);
            this->maxTemp = max(this->maxTemp, temperature);
        }
    }

    uint8_t vTick = max(abs((this->maxTemp - this->minTemp) / 3), 2);

    for (uint8_t ix = PLOT_X_LEFT; ix < PLOT_X_RIGHT; ix += 4)
    {
        for (int8_t t = this->minTemp; t <= this->maxTemp; t += vTick)
        {
            Peripherals::oled->drawPixel(ix, temperatrureToYOffset(t), SSD1306_WHITE);
            Peripherals::oled->setCursor(0, temperatrureToYOffset(t) - (2 + (t == this->maxTemp ? -3 : 0)));
            Peripherals::oled->print(t);
        }
    }

    for (int ix = PLOT_X_LEFT; ix < PLOT_X_RIGHT; ix += PLOT_X_PIXELS / 12)
    {
        Peripherals::oled->drawLine(ix, PLOT_Y_BOTTOM - 1, ix, PLOT_Y_BOTTOM + 1, SSD1306_WHITE);
    }

    Peripherals::oled->display();

    float temperaturePointA = 0;

    uint16_t logPtr = Peripherals::eeprom->eeprom_read(EEPROM_LOG_PTR);

    bool overflow = false;

    for (int ix = LOG_LENGTH_BYTES - 1; ix > 0; ix--)
    {
        if (Peripherals::buttonA->isPressed() || Peripherals::buttonB->isPressed())
        {
            Status::abortLoop = true;
            return;
        }

        uint8_t readingPointer = (logPtr + ix) % LOG_LENGTH_BYTES;

        float temperaturePointB = (Peripherals::eeprom->eeprom_read(EEPROM_T_LOG_BASE + readingPointer) - 127) / 2;

        if (ix == LOG_LENGTH_BYTES - 1)
        {
            temperaturePointA = temperaturePointB;
            continue;
        }

        Peripherals::oled->drawLine(plotIndexToXOffset(ix + 1), temperatrureToYOffset(temperaturePointA), plotIndexToXOffset(ix), temperatrureToYOffset(temperaturePointB), SSD1306_WHITE);

        if (ix % 10 == 0)
        {
            Peripherals::oled->display();
        }

        if (temperaturePointA > this->maxTemp || temperaturePointA < this->minTemp)
        {
            overflow = true;
        }

        temperaturePointA = temperaturePointB;
    }

    Peripherals::oled->fillRect(0, 5, 50, 10, SSD1306_BLACK);

    if (overflow)
    {
        Peripherals::oled->setCursor(0, 5);
        Peripherals::oled->print("OVFL");
    }

    Peripherals::oled->display();

    this->replotNeeded = false;
}
