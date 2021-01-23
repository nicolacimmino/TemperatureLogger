
#ifndef __TEMPERATURE_LOGGER_DISPLAY_H__
#define __TEMPERATURE_LOGGER_DISPLAY_H__

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Sodaq_SHT2x.h>
#include <uRTCLib.h>
#include <uEEPROMLib.h>

#include "config.h"
#include "PowerManager.h"
#include "Status.h"
#include "ui.h"

class Display
{
private:
protected:
    Adafruit_SSD1306 *oled;
    void clearDisplay();
    void plotBatterLevel();

public:
    Display(Adafruit_SSD1306 *oled);
    virtual void loop() = 0;
    virtual void onBClick() = 0;
};

#endif