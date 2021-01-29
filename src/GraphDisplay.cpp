#include "GraphDisplay.h"

void GraphDisplay::onBClick()
{
    this->plotAutoscale = !this->plotAutoscale;
}

void GraphDisplay::onBLongPress()
{
    return;
}

void GraphDisplay::loop()
{
    if (PowerManager::level != PS_LEVEL_0 || Status::shouldAbortLoop() || !this->replotNeeded)
    {
        return;
    }

    this->plot();
}

uint8_t GraphDisplay::valueToYOffset(float value)
{
    value = max(min(this->maxVal, value), this->minVal);
    return PLOT_Y_TOP + PLOT_Y_PIXELS - floor((PLOT_Y_PIXELS * (value - this->minVal)) / (this->maxVal - this->minVal));
}

uint8_t GraphDisplay::plotIndexToXOffset(uint8_t ix)
{
    // This is simplified as the log has as many points as the graph width.
    return ix + PLOT_X_LEFT;
}

uint16_t GraphDisplay::getStoredValue(uint8_t ix)
{    
    return Peripherals::eeprom->eeprom_read(EEPROM_T_LOG_BASE + (ix * LOG_ENTRY_BYTES)) << 8 | Peripherals::eeprom->eeprom_read(EEPROM_T_LOG_BASE + (ix * LOG_ENTRY_BYTES) + 1);
}

void GraphDisplay::plot()
{

    this->clearDisplay();

    Peripherals::oled->setCursor(0, 5);
    Peripherals::oled->print(F("BUSY...."));
    Peripherals::oled->display();

    Peripherals::oled->drawLine(PLOT_X_LEFT, PLOT_Y_BOTTOM, PLOT_X_RIGHT, PLOT_Y_BOTTOM, SSD1306_WHITE);
    Peripherals::oled->drawLine(PLOT_X_LEFT, PLOT_Y_BOTTOM, PLOT_X_LEFT, PLOT_Y_TOP, SSD1306_WHITE);

    this->setDefaultRange();

    if (this->plotAutoscale)
    {
        this->minVal = 100;
        this->maxVal = -100;
        for (int ix = 0; ix < LOG_LENGTH_POINTS; ix++)
        {
            if (Status::shouldAbortLoop())
            {
                return;
            }

            float value = this->getValueFromRawRecord(this->getStoredValue(ix));
            this->minVal = min(this->minVal, value);
            this->maxVal = max(this->maxVal, value);
        }
    }

    uint8_t vTick = this->getVTick();

    for (uint8_t ix = PLOT_X_LEFT; ix < PLOT_X_RIGHT; ix += 4)
    {
        for (int8_t v = this->minVal; v <= this->maxVal; v += vTick)
        {
            Peripherals::oled->drawPixel(ix, this->valueToYOffset(v), SSD1306_WHITE);
            Peripherals::oled->setCursor(0, this->valueToYOffset(v) - (2 + (v == this->maxVal ? -3 : 0)));
            Peripherals::oled->print(v);
        }
    }

    for (int ix = PLOT_X_LEFT; ix < PLOT_X_RIGHT; ix += PLOT_X_PIXELS / 12)
    {
        Peripherals::oled->drawLine(ix, PLOT_Y_BOTTOM - 1, ix, PLOT_Y_BOTTOM + 1, SSD1306_WHITE);
    }

    Peripherals::oled->display();

    float pointA = 0;

    uint16_t logPtr = Peripherals::eeprom->eeprom_read(EEPROM_LOG_PTR);

    bool overflow = false;

    for (int ix = LOG_LENGTH_POINTS - 1; ix > 0; ix--)
    {
        if (Status::shouldAbortLoop())
        {
            return;
        }

        uint8_t readingPointer = (logPtr + ix) % LOG_LENGTH_POINTS;

        float pointB = this->getValueFromRawRecord(this->getStoredValue(readingPointer));

        if (ix == LOG_LENGTH_POINTS - 1)
        {
            pointA = pointB;
            continue;
        }

        Peripherals::oled->drawLine(this->plotIndexToXOffset(ix + 1), this->valueToYOffset(pointA), this->plotIndexToXOffset(ix), this->valueToYOffset(pointB), SSD1306_WHITE);

        if (ix % 10 == 9)
        {
            Peripherals::oled->display();
        }

        if (pointA > this->maxVal || pointA < this->minVal)
        {
            overflow = true;
        }

        pointA = pointB;
    }

    Peripherals::oled->fillRect(0, 5, 50, 10, SSD1306_BLACK);

    if (overflow)
    {
        Peripherals::oled->setCursor(0, 5);
        Peripherals::oled->print(F("OVFL"));
    }

    Peripherals::oled->display();

    this->replotNeeded = false;
}
