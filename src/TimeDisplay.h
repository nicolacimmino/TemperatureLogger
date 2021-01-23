
#ifndef __TEMPERATURE_LOGGER_TIME_DISPLAY_H__
#define __TEMPERATURE_LOGGER_TIME_DISPLAY_H__

#include "Display.h"

class TimeDisplay : public Display
{
private:
    uRTCLib *rtc;
    void displayTime();

public:
    TimeDisplay(Adafruit_SSD1306 *oled, uRTCLib *rtc);
    void loop();
};

#endif