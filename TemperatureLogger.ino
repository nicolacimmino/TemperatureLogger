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
#include "uEEPROMLib.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Sodaq_SHT2x.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define PLOT_Y_MAX_C 30.0
#define PLOT_Y_MIN_C 15.0
#define PLOT_Y_TOP 16
#define PLOT_Y_BOTTOM (SCREEN_HEIGHT - 5)
#define PLOT_Y_PIXELS (PLOT_Y_BOTTOM - PLOT_Y_TOP)
#define PLOT_X_LEFT 12
#define PLOT_X_RIGHT (SCREEN_WIDTH - 4)
#define PLOT_X_PIXELS (PLOT_X_RIGHT - PLOT_X_LEFT)
#define RAMLOG_LENGTH_BYTES PLOT_X_PIXELS
#define LOG_LENGTH_BYTES 128
#define DISPLAY_I2C_ADDRESS 0x3C
#define TEMPERATURE_NOT_SET 0xFF
#define PIN_BUTTON_A 6
#define DISPLAY_MODES 2
#define POWER_SAVE_TIMEOUT_MS 20000
#define RECORD_DATA_INTERVAL_MS 300000 // 5 minutes

uRTCLib *rtc;
Adafruit_SSD1306 *oled;
uEEPROMLib *eeprom;
uint8_t mode = 0;
uint16_t logStartPointer = 0;
uint16_t logEndPointer = 0;
uint8_t ramLog[RAMLOG_LENGTH_BYTES];
uint8_t ramLogPointer = 0;
unsigned long lastButtonActivityTime = millis();
bool powerSaveOn = false;

uint8_t getBatteryLevel()
{
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

    return min(max((measuredVcc - 2700) / 7, 0), 100);
}

void displayBatterLevel()
{
    oled->setCursor(100, 5);
    oled->print(getBatteryLevel());
    oled->print("%");
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

    uint8_t temperatureEncoded = 127 + (SHT2x.GetTemperature() * 2);

    // eeprom->eeprom_write(logEndPointer, temperatureEncoded);
    // logEndPointer = (logEndPointer + 1) % LOG_LENGTH_BYTES;

    ramLog[ramLogPointer] = temperatureEncoded;
    ramLogPointer = (ramLogPointer + 1) % RAMLOG_LENGTH_BYTES;
}

uint8_t temperatrureToYOffset(float temperature)
{
    temperature = max(min(PLOT_Y_MAX_C, temperature), PLOT_Y_MIN_C);
    return PLOT_Y_TOP + PLOT_Y_PIXELS - floor((PLOT_Y_PIXELS * (temperature - PLOT_Y_MIN_C)) / (PLOT_Y_MAX_C - PLOT_Y_MIN_C));
}

uint8_t plotIndexToXOffset(uint8_t ix)
{
    // This is simplified as the RAM shadow of the log is on purpose as
    // many points as the plot pixels.
    return ix + PLOT_X_LEFT;
}

void plotTemperature()
{
    clearDisplay();

    oled->drawLine(PLOT_X_LEFT, PLOT_Y_BOTTOM, PLOT_X_RIGHT, PLOT_Y_BOTTOM, SSD1306_WHITE);
    oled->drawLine(PLOT_X_LEFT, PLOT_Y_BOTTOM, PLOT_X_LEFT, PLOT_Y_TOP, SSD1306_WHITE);

    for (int ix = PLOT_X_LEFT; ix < PLOT_X_RIGHT; ix += 4)
    {
        oled->drawPixel(ix, temperatrureToYOffset(20), SSD1306_WHITE);
        oled->drawPixel(ix, temperatrureToYOffset(25), SSD1306_WHITE);
        oled->drawPixel(ix, temperatrureToYOffset(30), SSD1306_WHITE);
        oled->setCursor(0, temperatrureToYOffset(15) - 3);
        oled->print(15);
        oled->setCursor(0, temperatrureToYOffset(20) - 3);
        oled->print(20);
        oled->setCursor(0, temperatrureToYOffset(25) - 3);
        oled->print(25);
        oled->setCursor(0, temperatrureToYOffset(30));
        oled->print(30);
    }

    for (int ix = PLOT_X_LEFT; ix < PLOT_X_RIGHT; ix += PLOT_X_PIXELS / 12)
    {
        oled->drawLine(ix, PLOT_Y_BOTTOM - 1, ix, PLOT_Y_BOTTOM + 1, SSD1306_WHITE);
    }

    for (int ix = 0; ix < RAMLOG_LENGTH_BYTES; ix++)
    {
        uint8_t readingPointer = (ramLogPointer + ix) % RAMLOG_LENGTH_BYTES;

        if (readingPointer == 0 || ramLog[readingPointer] == TEMPERATURE_NOT_SET || ramLog[readingPointer - 1] == TEMPERATURE_NOT_SET)
        {
            continue;
        }

        int8_t temperaturePointA = (ramLog[readingPointer - 1] - 127) / 2;
        int8_t temperaturePointB = (ramLog[readingPointer] - 127) / 2;
        oled->drawLine(plotIndexToXOffset(ix), temperatrureToYOffset(temperaturePointA), plotIndexToXOffset(ix + 1), temperatrureToYOffset(temperaturePointB), SSD1306_WHITE);
    }

    oled->display();
}

void setup()
{
    Wire.begin();
    Serial.begin(9600);

    rtc = new uRTCLib(0x68);

    //eeprom = new uEEPROMLib(0x57);

    // Initialize to TEMPERATURE_NOT_SET which is not plotted.
    // This later will be a copy from the last RAMLOG_LENGTH_BYTES in EEPROM.
    memset(ramLog, TEMPERATURE_NOT_SET, RAMLOG_LENGTH_BYTES);

    pinMode(PIN_BUTTON_A, INPUT_PULLUP);

    oled = new Adafruit_SSD1306(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
    oled->begin(SSD1306_SWITCHCAPVCC, DISPLAY_I2C_ADDRESS);
    oled->clearDisplay();
    oled->display();
}

void managePowerSave()
{
    if (powerSaveOn)
    {
        return;
    }

    if (millis() - lastButtonActivityTime > POWER_SAVE_TIMEOUT_MS)
    {
        powerSaveOn = true;
        oled->ssd1306_command(SSD1306_DISPLAYOFF);
    }
}

void exitPowerSave()
{
    powerSaveOn = false;
    oled->ssd1306_command(SSD1306_DISPLAYON);
}

void checkButtonA()
{
    if (digitalRead(PIN_BUTTON_A) == LOW)
    {
        lastButtonActivityTime = millis();

        while (digitalRead(PIN_BUTTON_A) == LOW)
        {
            delay(1);
        }

        if (powerSaveOn)
        {
            exitPowerSave();
            return;
        }

        mode = (mode + 1) % DISPLAY_MODES;
    }
}

void loop()
{
    recordData();
    managePowerSave();
    checkButtonA();

    if (powerSaveOn)
    {
        return;
    }

    switch (mode)
    {
    case 0:
        displayTime();
        break;
    case 1:
        plotTemperature();
        break;
    }

    delay(500);
}
