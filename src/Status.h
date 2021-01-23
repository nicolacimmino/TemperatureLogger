#ifndef __TEMPERATURE_STATUS_H__
#define __TEMPERATURE_STATUS_H__

#include <Arduino.h>

class Status
{
private:
    static unsigned long lastTimeSync;

public:
    static bool isTimeSynced();
    static void timeSynced();
    static uint8_t getBatteryLevel();
    static bool replotNeeded;
};

#endif