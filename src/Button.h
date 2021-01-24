#ifndef __TEMPERATURE_LOGGER_BUTTON_H__
#define __TEMPERATURE_LOGGER_BUTTON_H__

#include <Arduino.h>

class Button
{
private:
    uint8_t pin;
    void (*onClick)();
    void (*onLongPress)();

public:
    Button(uint8_t pin);
    void registerOnClickCallback(void (*callback)());
    void registerOnLongPressCallback(void (*callback)());
    void loop();
    bool isPressed();
};

#endif
