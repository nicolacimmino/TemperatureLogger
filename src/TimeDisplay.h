
#ifndef __TEMPERATURE_LOGGER_TIME_DISPLAY_H__
#define __TEMPERATURE_LOGGER_TIME_DISPLAY_H__

#include "Display.h"

class TimeDisplay : public Display
{
private:
    uRTCLib *rtc;
    void displayTime();

public:    
    void loop();
    void onBClick();    
};

#endif