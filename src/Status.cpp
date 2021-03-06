#include "Status.h"

unsigned long Status::lastTimeSync = 0;
long Status::batteryVoltage = 0;
bool Status::abortLoopRequested = false;
bool Status::timeSyncRequsted = true;

static void Status::abortLoop()
{
    Status::abortLoopRequested = true;
}

static bool Status::shouldAbortLoop()
{
    return Status::abortLoopRequested;
}

static void Status::loopAborted()
{
    Status::abortLoopRequested = false;
}

bool Status::timeSync()
{
    Status::timeSyncRequsted = true;
}

bool Status::shouldTimeSync()
{
    return Status::timeSyncRequsted;
}

bool Status::isTimeSynced()
{
    return (lastTimeSync > 0) && (millis() - lastTimeSync) < 86400000;
}

void Status::timeSynced()
{
    lastTimeSync = millis();
    Status::timeSyncRequsted = false;
}

void Status::timeSyncFailed()
{
    Status::timeSyncRequsted = false;
}

long Status::getBatteryVoltage()
{
    return Status::batteryVoltage;
}

uint8_t Status::getBatteryLevel()
{
    static uint8_t batteryLevel = 0;
    static unsigned long lastMeasurementTime = 0;

    if (batteryLevel != 0 && millis() - lastMeasurementTime < 1000)
    {
        return batteryLevel;
    }

    lastMeasurementTime = millis();

    // See this article for an in-depth explanation.
    // https://provideyourown.com/2012/secret-arduino-voltmeter-measure-battery-voltage/
    // tl;dr: we switch the ADC to measure the internal 1.1v reference using Vcc as reference, the rest is simple math.

    ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);

    delay(2);
    ADCSRA |= _BV(ADSC);
    while (bit_is_set(ADCSRA, ADSC))
        ;

    Status::batteryVoltage = 1125300L / (ADCL | (ADCH << 8));
    analogReference(DEFAULT);

    // We assume 3900mV max and 2900 being the safe discharge level. 3900-2900 => 1000
    // 1000 / 10 => 100 (%).
    uint8_t measuredLevel = min(max((Status::batteryVoltage - 2900) / 10, 0), 100);

    // Init the IIR filter with the first sample otherwise the % indicator will ramp up slowly at power on.
    if (batteryLevel == 0)
    {
        batteryLevel = measuredLevel;
    }

    batteryLevel = (0.9 * (float)batteryLevel) + (0.1 * (float)measuredLevel);

    return batteryLevel;
}

/**
 * See https://playground.arduino.cc/Code/AvailableMemory/
 **/
int Status::getFreeRamBytes()
{
    extern int __heap_start, *__brkval;
    int v;
    return (int)&v - (__brkval == 0 ? (int)&__heap_start : (int)__brkval);
}