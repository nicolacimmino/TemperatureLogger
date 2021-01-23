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
#include <uRTCLib.h>
#include <uEEPROMLib.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Sodaq_SHT2x.h>
#include <DCServices.h>

#include "src/Button.h"
#include "src/TimeDisplay.h"
#include "src/TemperatureDisplay.h"
#include "src/Peripherals.h"

Display *currentDisplay = NULL;
uint8_t mode = 0;

void recordData()
{
    static unsigned long lastRecord = millis();
    if (millis() - lastRecord < RECORD_DATA_INTERVAL_MS)
    {
        return;
    }
    lastRecord = millis();

    bool restoreLevel = false;
    if (PowerManager::level > PS_LEVEL_1)
    {
        PowerManager::enterL1();
        restoreLevel = true;
    }

    uint8_t temperatureEncoded = 127 + (SHT2x.GetTemperature() * 2);

    uint8_t logPtr = Peripherals::eeprom->eeprom_read(EEPROM_LOG_PTR);
    Peripherals::eeprom->eeprom_write(EEPROM_T_LOG_BASE + logPtr, temperatureEncoded);
    logPtr = (logPtr + 1) % LOG_LENGTH_BYTES;
    Peripherals::eeprom->eeprom_write(EEPROM_LOG_PTR, logPtr);

    currentDisplay->onDisplayInvalidated();

    if (restoreLevel)
    {
        PowerManager::restoreLevel();
    }
}

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
    }
}

void changeMode()
{
    mode = (mode + 1) % DISPLAY_MODES;
    enterMode();
}
void onButtonAClick()
{
    if (PowerManager::level == PS_LEVEL_0)
    {
        changeMode();
    }

    PowerManager::onUserInteratcion();

    currentDisplay->onDisplayInvalidated();
    currentDisplay->loop();
}

void onButtonBClick()
{
    if (PowerManager::level == PS_LEVEL_0)
    {
        currentDisplay->onBClick();
    }

    PowerManager::onUserInteratcion();

    currentDisplay->onDisplayInvalidated();
    currentDisplay->loop();
}

void setup()
{
    Serial.begin(9600);

    pinMode(PIN_PWR_AUX_DEVS, OUTPUT);
    digitalWrite(PIN_PWR_AUX_DEVS, HIGH);

    Wire.begin();

    Peripherals::rtc = new uRTCLib(0x68);

    DCServices *dcServices = new DCServices(DC_RADIO_NRF24_V2, Peripherals::rtc);
    if (dcServices->syncRTCToTimeBroadcast())
    {
        Status::timeSynced();
    }
    delete dcServices;

    Peripherals::eeprom = new uEEPROMLib(0x57);

    Peripherals::buttonA = new Button(PIN_BUTTON_A);
    Peripherals::buttonA->registerOnClickCallback(onButtonAClick);
    Peripherals::buttonB = new Button(PIN_BUTTON_B);
    Peripherals::buttonB->registerOnClickCallback(onButtonBClick);

    Peripherals::oled = new Adafruit_SSD1306(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
    Peripherals::oled->begin(SSD1306_SWITCHCAPVCC, DISPLAY_I2C_ADDRESS);
    Peripherals::oled->clearDisplay();
    Peripherals::oled->display();

    enterMode();
}

void loop()
{
    PowerManager::loop();
    recordData();
    Peripherals::buttonA->loop();
    Peripherals::buttonB->loop();
    currentDisplay->loop();
}
