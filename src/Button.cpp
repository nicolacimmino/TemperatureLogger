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

bool Button::isPressed()
{
    return digitalRead(this->pin) == LOW;
}

void Button::loop()
{
    if (!this->isPressed() || Status::shouldAbortLoop())
    {
        return;
    }

    // Allow the switch to stabilise.
    byte debounce = 0x55;
    while (debounce != 0x00)
    {
        debounce = (debounce << 1) | (this->isPressed() ? 0 : 1);
        delay(1);
    }

    // Wait for the switch to be released or a timeout of 500mS to expire.
    unsigned long initialTime = millis();
    while ((millis() - initialTime < 500) && this->isPressed())
    {
        delay(1);
    }

    if (this->isPressed() && this->onLongPress != NULL)
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

    while (this->isPressed() && !Status::shouldAbortLoop())
    {
        delay(1);
    }
}
