
#ifndef __TEMPERATURE_LOGGER_DISPLAY_H__
#define __TEMPERATURE_LOGGER_DISPLAY_H__

#include "config.h"
#include "PowerManager.h"
#include "Status.h"
#include "ui.h"
#include "Peripherals.h"

class Display
{
private:
protected:
    void clearDisplay();
    void plotBatterLevel();
    bool replotNeeded = true;
    
public:
    virtual void loop() = 0;
    virtual void onBClick() = 0;
    virtual void onBLongPress() = 0;    
    void onDisplayInvalidated();
};

#endif