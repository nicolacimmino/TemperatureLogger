//
//  Temperature logger.
//
//  Copyright (C) 2019 Nicola Cimmino
//
//    This program is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//   This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program.  If not, see http://www.gnu.org/licenses/.
//
#include <Arduino.h>
#include <avr/sleep.h>
#include <Wire.h>

#include "src/Button.h"
#include "src/TimeDisplay.h"
#include "src/TemperatureDisplay.h"
#include "src/HumidityDisplay.h"
#include "src/StatusDisplay.h"
#include "src/Peripherals.h"
#include "src/DataStore.h"

Display *currentDisplay = NULL;
uint8_t mode = 0;

void enterMode()
{

    if (currentDisplay != NULL)
    {
        delete currentDisplay;
    }

    switch (mode)
    {
    case 0:
        currentDisplay = new TimeDisplay();
        break;
    case 1:
        currentDisplay = new TemperatureDisplay();
        break;
    case 2:
        currentDisplay = new HumidityDisplay();
        break;
    case 3:
        currentDisplay = new StatusDisplay();
        break;
    }
}

void changeMode()
{
    mode = (mode + 1) % DISPLAY_MODES;
    enterMode();
}

void onButtonPress()
{
    PowerManager::onUserInteratcion();

    currentDisplay->onDisplayInvalidated();
    currentDisplay->loop();
}

void onButtonAClick()
{
    if (PowerManager::level == PS_LEVEL_0)
    {
        changeMode();
    }

    onButtonPress();
}

void onButtonBClick()
{
    if (PowerManager::level == PS_LEVEL_0)
    {
        currentDisplay->onBClick();
    }

    onButtonPress();
}

void onButtonBLongPress()
{
    if (PowerManager::level == PS_LEVEL_0)
    {
        currentDisplay->onBLongPress();
    }

    onButtonPress();
}

void buttonPressedISR()
{
    static unsigned long lastInterruptTime = 0;

    if (millis() - lastInterruptTime > 200)
    {
        Status::abortLoop();
    }
    lastInterruptTime = millis();
}

void setup()
{
    Serial.begin(9600);

    pinMode(PIN_PWR_AUX_DEVS, OUTPUT);
    digitalWrite(PIN_PWR_AUX_DEVS, HIGH);

    Wire.begin();

    Peripherals::setup();
    
    if (Peripherals::buttonA->isPressed() && Peripherals::buttonB->isPressed())
    {
        DataStore::wipeStoredData();
    }

    attachInterrupt(digitalPinToInterrupt(PIN_BUTTON_A), buttonPressedISR, FALLING);
    attachInterrupt(digitalPinToInterrupt(PIN_BUTTON_B), buttonPressedISR, FALLING);

    Peripherals::buttonA->registerOnClickCallback(onButtonAClick);
    Peripherals::buttonB->registerOnClickCallback(onButtonBClick);
    Peripherals::buttonB->registerOnLongPressCallback(onButtonBLongPress);

    enterMode();
}

void loop()
{
    PowerManager::loop();
    if (DataStore::recordData())
    {
        currentDisplay->onDisplayInvalidated();
    }
    Peripherals::buttonA->loop();
    Peripherals::buttonB->loop();
    currentDisplay->loop();

    if (Status::shouldAbortLoop())
    {
        Status::loopAborted();

        return;
    }

    if (Status::shouldTimeSync())
    {
        if (Peripherals::dcServices->syncRTCToTimeBroadcast())
        {
            Status::timeSynced();
        }
        else
        {
            Status::timeSyncFailed();
        }
    }
}
