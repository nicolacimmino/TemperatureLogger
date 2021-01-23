#include "Button.h"

Button::Button(uint8_t pin)
{
    this->pin = pin;
    pinMode(this->pin, INPUT_PULLUP);
}

void Button::registerOnClickCallback(void (*callback)())
{
    this->onClick = callback;
}

void Button::registerOnLongPressCallback(void (*callback)())
{
    this->onLongPress = callback;
}

void Button::loop()
{
    if (digitalRead(this->pin) != LOW)
    {
        return;
    }

    // Allow the switch to stabilise.
    byte debounce = 0x55;
    while (debounce != 0x00)
    {
        debounce = (debounce << 1) | (digitalRead(this->pin) & 1);
        delay(1);
    }

    // Wait for the switch to be released or a timeout of 500mS to expire.
    unsigned long initialTime = millis();
    while ((millis() - initialTime < 500) && digitalRead(this->pin) == LOW)
    {
        delay(1);
    }

    if (digitalRead(this->pin) == LOW)
    {
        if (this->onLongPress != NULL)
        {
            this->onLongPress();
        }
    }
    else
    {
        if (this->onClick != NULL)
        {
            this->onClick();
        }
    }

    while (digitalRead(this->pin) == LOW)
    {
        delay(1);
    }
}
