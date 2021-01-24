#ifndef __TEMPERATURE_STATUS_H__
#define __TEMPERATURE_STATUS_H__

#include <Arduino.h>

class Status
{
private:
    static unsigned long lastTimeSync;
    static long batteryVoltage;
    static bool abortLoopRequested;
    static bool timeSyncRequsted;

public:
    static void abortLoop();
    static bool shouldAbortLoop();
    static void loopAborted();
    static bool shouldTimeSync();
    static bool isTimeSynced();
    static void timeSynced();
    static void timeSyncFailed();
    static uint8_t getBatteryLevel();
    static long getBatteryVoltage();
    static int getFreeRamBytes();
};

#endif