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
#include "src/Peripherals.h"
#include "src/ModeManager.h"
#include "src/DataStore.h"


void onButtonPress()
{
    PowerManager::onUserInteratcion();
    
    ModeManager::currentDisplay->loop();
}

void onButtonAClick()
{
    if (PowerManager::level == PS_LEVEL_0)
    {
        ModeManager::changeMode();
    }

    onButtonPress();
}

void onButtonBClick()
{
    if (PowerManager::level == PS_LEVEL_0)
    {
        ModeManager::currentDisplay->onBClick();
    }

    onButtonPress();
}

void onButtonBLongPress()
{
    if (PowerManager::level == PS_LEVEL_0)
    {
        ModeManager::currentDisplay->onBLongPress();
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

    ModeManager::setup();
}

void loop()
{
    PowerManager::loop();
    DataStore::loop();
    Peripherals::buttonA->loop();
    Peripherals::buttonB->loop();
    ModeManager::currentDisplay->loop();

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
