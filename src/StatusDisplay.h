
#ifndef __TEMPERATURE_LOGGER_STATUS_DISPLAY_H__
#define __TEMPERATURE_LOGGER_STATUS_DISPLAY_H__

#include "Display.h"

class StatusDisplay : public Display
{
private:
    void displayStatus();
    
public:
    void loop();
    void onBClick();
};

#endif