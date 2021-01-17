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

//#define __ENABLE_DC_SERVICES__ 1

#include <Arduino.h>
#include <avr/sleep.h>
#include <Wire.h>
#include <uRTCLib.h>
#include <uEEPROMLib.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Sodaq_SHT2x.h>

#include "config.h"
#include "ui.h"
#include "powerManager.h"

#ifdef __ENABLE_DC_SERVICES__
#include <DCServices.h>
DCServices *dcServices;
#endif

uRTCLib *rtc;
Adafruit_SSD1306 *oled;
uEEPROMLib *eeprom;

uint8_t mode = 0;
bool plotAutoscale = false;
bool replotNeeded = true;

uint8_t getBatteryLevel()
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

    long measuredVcc = 1125300L / (ADCL | (ADCH << 8));
    analogReference(DEFAULT);

    // We assume 3900mV max and 2900 being the safe discharge level. 3900-2900 => 1000
    // 1000 / 10 => 100 (%).
    uint8_t measuredLevel = min(max((measuredVcc - 2900) / 10, 0), 100);

    // Init the IIR filter with the first sample otherwise the % indicator will ramp up slowly at power on.
    if (batteryLevel == 0)
    {
        batteryLevel = measuredLevel;
    }

    batteryLevel = (0.9 * (float)batteryLevel) + (0.1 * (float)measuredLevel);

    return batteryLevel;
}

void displayBatterLevel()
{
    uint8_t batteryLevel = getBatteryLevel();
    oled->setCursor(80, 5);
    oled->print(batteryLevel);
    oled->print("%");

    oled->drawBitmap(110, 5, batteryLogo[round(batteryLevel / 25.0)], BATTERY_LOGO_W, BATTERY_LOGO_H, SSD1306_WHITE);
}

void clearDisplay()
{
    oled->clearDisplay();
    oled->setTextSize(1);
    oled->setTextColor(SSD1306_WHITE);
    displayBatterLevel();
}

void displayTime()
{
    char text[16];
    char textB[6];

    rtc->refresh();

    clearDisplay();

    sprintf(text, "%02i:%02i:%02i", rtc->hour(), rtc->minute(), rtc->second());
    oled->setTextSize(2);
    oled->setCursor(18, 21);
    oled->print(text);

    float temperature = (SHT2x.GetTemperature() * 10) / 10.0;
    float humidity = round(SHT2x.GetHumidity());

    sprintf(text, "%sC", dtostrf(temperature, 3, 1, textB));
    oled->setTextSize(1);
    oled->setCursor(0, 50);
    oled->print(text);

    sprintf(text, "%s%%", dtostrf(humidity, 3, 0, textB));
    oled->setCursor(103, 50);
    oled->print(text);

    oled->display();
}

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

    uint8_t logPtr = eeprom->eeprom_read(EEPROM_LOG_PTR);
    eeprom->eeprom_write(EEPROM_T_LOG_BASE + logPtr, temperatureEncoded);
    logPtr = (logPtr + 1) % LOG_LENGTH_BYTES;
    eeprom->eeprom_write(EEPROM_LOG_PTR, logPtr);

    replotNeeded = true;

    if (restoreLevel)
    {
        PowerManager::restoreLevel();
    }
}

uint8_t temperatrureToYOffset(float temperature, int8_t minTemp, int8_t maxTemp)
{
    temperature = max(min(maxTemp, temperature), minTemp);
    return PLOT_Y_TOP + PLOT_Y_PIXELS - floor((PLOT_Y_PIXELS * (temperature - minTemp)) / (maxTemp - minTemp));
}

uint8_t plotIndexToXOffset(uint8_t ix)
{
    // This is simplified as the RAM shadow of the log is on purpose as
    // many points as the plot pixels.
    return ix + PLOT_X_LEFT;
}

void plotTemperature()
{
    if (!replotNeeded)
    {
        return;
    }

    clearDisplay();

    oled->setCursor(0, 5);
    oled->print("BUSY....");
    oled->display();

    oled->drawLine(PLOT_X_LEFT, PLOT_Y_BOTTOM, PLOT_X_RIGHT, PLOT_Y_BOTTOM, SSD1306_WHITE);
    oled->drawLine(PLOT_X_LEFT, PLOT_Y_BOTTOM, PLOT_X_LEFT, PLOT_Y_TOP, SSD1306_WHITE);

    int8_t minTemp = 100;
    int8_t maxTemp = -100;

    if (plotAutoscale)
    {
        for (int ix = LOG_LENGTH_BYTES - 1; ix > 0; ix--)
        {
            int8_t temperature = (eeprom->eeprom_read(EEPROM_T_LOG_BASE + ix) - 127) / 2;
            minTemp = min(minTemp, temperature);
            maxTemp = max(maxTemp, temperature);
        }
    }
    else
    {
        minTemp = 15;
        maxTemp = 30;
    }

    uint8_t hTick = max(abs((maxTemp - minTemp) / 3), 1);

    for (uint8_t ix = PLOT_X_LEFT; ix < PLOT_X_RIGHT; ix += 4)
    {
        for (int8_t t = minTemp; t <= maxTemp; t += hTick)
        {
            oled->drawPixel(ix, temperatrureToYOffset(t, minTemp, maxTemp), SSD1306_WHITE);
            oled->setCursor(0, temperatrureToYOffset(t, minTemp, maxTemp) - (2 + (t == maxTemp ? -3 : 0)));
            oled->print(t);
        }
    }

    for (int ix = PLOT_X_LEFT; ix < PLOT_X_RIGHT; ix += PLOT_X_PIXELS / 12)
    {
        oled->drawLine(ix, PLOT_Y_BOTTOM - 1, ix, PLOT_Y_BOTTOM + 1, SSD1306_WHITE);
    }

    oled->display();

    int8_t temperaturePointA = 0;

    uint16_t logPtr = eeprom->eeprom_read(EEPROM_LOG_PTR);

    bool overflow = false;

    for (int ix = LOG_LENGTH_BYTES - 1; ix > 0; ix--)
    {
        uint8_t readingPointer = (logPtr + ix) % LOG_LENGTH_BYTES;

        int8_t temperaturePointB = (eeprom->eeprom_read(EEPROM_T_LOG_BASE + readingPointer) - 127) / 2;

        if (ix == LOG_LENGTH_BYTES - 1)
        {
            temperaturePointA = temperaturePointB;
            continue;
        }

        oled->drawLine(plotIndexToXOffset(ix + 1), temperatrureToYOffset(temperaturePointA, minTemp, maxTemp), plotIndexToXOffset(ix), temperatrureToYOffset(temperaturePointB, minTemp, maxTemp), SSD1306_WHITE);

        if (ix % 10 == 0)
        {
            oled->display();
        }

        if (temperaturePointA > maxTemp || temperaturePointA < minTemp)
        {
            overflow = true;
        }

        temperaturePointA = temperaturePointB;
    }

    oled->fillRect(0, 5, 50, 10, SSD1306_BLACK);

    if (overflow)
    {
        oled->setCursor(0, 5);
        oled->print("OVFL");
    }

    oled->display();

    replotNeeded = false;
}

void setup()
{
    Serial.begin(9600);

    pinMode(PIN_PWR_AUX_DEVS, OUTPUT);
    digitalWrite(PIN_PWR_AUX_DEVS, HIGH);

    Wire.begin();

    rtc = new uRTCLib(0x68);

#ifdef __ENABLE_DC_SERVICES__
    delay(100);
    dcServices = new DCServices(DC_RADIO_NRF24_V2, rtc);
    dcServices->syncRTCToTimeBroadcast();    
#endif

    eeprom = new uEEPROMLib(0x57);

    pinMode(PIN_BUTTON_A, INPUT_PULLUP);
    pinMode(PIN_BUTTON_B, INPUT_PULLUP);

    oled = new Adafruit_SSD1306(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
    oled->begin(SSD1306_SWITCHCAPVCC, DISPLAY_I2C_ADDRESS);
    oled->clearDisplay();
    oled->display();
}

void checkButtonA()
{
    if (digitalRead(PIN_BUTTON_A) == LOW)
    {
        if (PowerManager::level == PS_LEVEL_0)
        {
            mode = (mode + 1) % DISPLAY_MODES;
        }

        PowerManager::onUserInteratcion();

        replotNeeded = true;
        plotAutoscale = false;
        serveScreen();

        while (digitalRead(PIN_BUTTON_A) == LOW)
        {
            delay(1);
        }
    }
}

void checkButtonB()
{
    if (digitalRead(PIN_BUTTON_B) == LOW)
    {
        if (PowerManager::level == PS_LEVEL_0)
        {
            plotAutoscale = !plotAutoscale;
        }

        PowerManager::onUserInteratcion();

        replotNeeded = true;
        serveScreen();

        while (digitalRead(PIN_BUTTON_B) == LOW)
        {
            delay(1);
        }
    }
}

void serveScreen()
{
    switch (mode)
    {
    case 0:
        displayTime();
        break;
    case 1:
        plotTemperature();
        break;
    }
}

void loop()
{    
    PowerManager::loop();
    recordData();
    checkButtonA();
    checkButtonB();

    if (PowerManager::level != PS_LEVEL_0)
    {
        return;
    }

    serveScreen();
}
