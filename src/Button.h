#ifndef __TEMPERATURE_LOGGER_BUTTON_H__
#define __TEMPERATURE_LOGGER_BUTTON_H__

#include <Arduino.h>
#include "Status.h"

class Button
{
private:
    uint8_t pin;
    void (*onClick)() = NULL;
    void (*onLongPress)() = NULL;    
public:
    Button(uint8_t pin);
    void registerOnClickCallback(void (*callback)());
    void registerOnLongPressCallback(void (*callback)());
    void loop();
    bool isPressed();
};

#endif
