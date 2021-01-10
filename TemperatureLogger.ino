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

#include <DCServices.h>
#include <uRTCLib.h>
#include "uEEPROMLib.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Fonts/TomThumb.h>
#include <Sodaq_SHT2x.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define PLOT_Y_MAX_C 30.0
#define PLOT_Y_MIN_C 15.0
#define PLOT_Y_TOP 10
#define PLOT_Y_BOTTOM (SCREEN_HEIGHT - 4)
#define PLOT_Y_PIXELS (PLOT_Y_BOTTOM - PLOT_Y_TOP)
#define PLOT_X_LEFT 4
#define PLOT_X_RIGHT (SCREEN_WIDTH - 4)
#define PLOT_X_PIXELS (PLOT_X_RIGHT - PLOT_X_LEFT)
#define RAMLOG_LENGTH_BYTES PLOT_X_PIXELS
#define LOG_LENGTH_BYTES 128
#define DISPLAY_I2C_ADDRESS 0x3C
#define TEMPERATURE_NOT_SET 0xFF

uRTCLib *rtc;
DCServices *dcServices;
Adafruit_SSD1306 *oled;
uEEPROMLib *eeprom;
bool timeSyncOK = false;
uint16_t logStartPointer = 0;
uint16_t logEndPointer = 0;
uint8_t ramLog[RAMLOG_LENGTH_BYTES];
uint8_t ramLogPointer = 0;

void displayTime()
{
    char text[16];
    char textB[6];

    rtc->refresh();

    oled->clearDisplay();
    oled->setTextSize(2);
    oled->setTextColor(SSD1306_WHITE);
    oled->cp437(true);

    sprintf(text, "%02i:%02i:%02i", rtc->hour(), rtc->minute(), rtc->second());
    oled->setCursor(18, 8);
    oled->print(text);

    float temperature = (SHT2x.GetTemperature() * 10) / 10.0;
    float humidity = round(SHT2x.GetHumidity());

    oled->setTextSize(1);
    sprintf(text, "%sC", dtostrf(temperature, 3, 1, textB));
    oled->setCursor(0, 25);
    oled->print(text);

    sprintf(text, "%s%%", dtostrf(humidity, 3, 0, textB));
    oled->setCursor(103, 25);
    oled->print(text);

    if (timeSyncOK)
    {
        oled->setCursor(0, 0);
        oled->print("TS");
    }

    oled->display();
}

void recordData()
{
    static unsigned long lastRecord = millis();
    if (millis() - lastRecord < 2000)
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
    // oled->clearDisplay();
    // oled->setFont(&TomThumb);
    // oled->setTextSize(1);
    // oled->setTextColor(SSD1306_WHITE);
    // oled->setCursor(0, 7);
    // oled->print("T x=24h y=15-30C");
    oled->drawLine(PLOT_X_LEFT, PLOT_Y_BOTTOM, PLOT_X_RIGHT, PLOT_Y_BOTTOM, SSD1306_WHITE);
    oled->drawLine(PLOT_X_LEFT, PLOT_Y_BOTTOM, PLOT_X_LEFT, PLOT_Y_TOP, SSD1306_WHITE);

    for (int ix = PLOT_X_LEFT; ix < PLOT_X_RIGHT; ix += 4)
    {
        oled->drawPixel(ix, temperatrureToYOffset(20), SSD1306_WHITE);
        oled->drawPixel(ix, temperatrureToYOffset(25), SSD1306_WHITE);
        oled->drawPixel(ix, temperatrureToYOffset(30), SSD1306_WHITE);
    }

    for (int ix = PLOT_X_LEFT; ix < PLOT_X_RIGHT; ix += PLOT_X_PIXELS / 12)
    {
        oled->drawLine(ix, PLOT_Y_BOTTOM - 1, ix, PLOT_Y_BOTTOM + 1, SSD1306_WHITE);
    }

    // for (int ix = 0; ix < RAMLOG_LENGTH_BYTES; ix++)
    // {
    //     uint8_t readingPointer = (ramLogPointer + ix) % RAMLOG_LENGTH_BYTES;

    //     if (readingPointer == 0 || ramLog[readingPointer] == TEMPERATURE_NOT_SET || ramLog[readingPointer - 1] == TEMPERATURE_NOT_SET)
    //     {
    //         continue;
    //     }

    //     int8_t temperaturePointA = (ramLog[readingPointer - 1] - 127) / 2;
    //     int8_t temperaturePointB = (ramLog[readingPointer] - 127) / 2;
    //     oled->drawLine(plotIndexToXOffset(ix), temperatrureToYOffset(temperaturePointA), plotIndexToXOffset(ix + 1), temperatrureToYOffset(temperaturePointB), SSD1306_WHITE);
    // }

    oled->display();
}

void setup()
{
    Wire.begin();
    Serial.begin(9600);

    rtc = new uRTCLib(0x68);

    eeprom = new uEEPROMLib(0x57);

    dcServices = new DCServices(DC_RADIO_NRF24_V2, rtc);

    oled = new Adafruit_SSD1306(SCREEN_WIDTH, 32, &Wire, -1);
    oled->begin(SSD1306_SWITCHCAPVCC, DISPLAY_I2C_ADDRESS);

    oled->clearDisplay();
    oled->display();

    timeSyncOK = dcServices->syncRTCToTimeBroadcast();

    // Initialize to TEMPERATURE_NOT_SET which is not plotted.
    // This later will be a copy from the last RAMLOG_LENGTH_BYTES in EEPROM.
    memset(ramLog, TEMPERATURE_NOT_SET, RAMLOG_LENGTH_BYTES);
}

void loop()
{
    plotTemperature();
    //displayTime();
    // if ((millis() / 1000) % 20 < 10)
    // {
    //     displayTime();
    // }
    // else
    // {
    //     plotTemperature();
    // }

    recordData();

    dcServices->loop();
}