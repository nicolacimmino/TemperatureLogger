
#ifndef __TEMPERATURE_LOGGER_TIME_DISPLAY_H__
#define __TEMPERATURE_LOGGER_TIME_DISPLAY_H__

#include "Display.h"

#define TIME_MODE_TIME 0
#define TIME_MODE_DATE 1
#define TIME_MODES_COUNT 2

#define TIME_REVERT_TO_DEFAULT_DELAY_MS 5000
#define TIME_SENSOR_READ_INTERVAL_MS 10000

class TimeDisplay : public Display
{
private:
    uRTCLib *rtc;
    void displayTime();
    uint8_t mode = TIME_MODE_TIME;
    unsigned long lastModeChangeTime = 0;
    unsigned long lastSensorReadingTime = 0;
    float temperature;
    float humidity;
public:    
    void loop();
    void onBClick();    
};

#endif