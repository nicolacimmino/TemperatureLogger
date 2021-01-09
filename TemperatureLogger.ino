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
#include <Sodaq_SHT2x.h>

uRTCLib *rtc;
DCServices *dcServices;
Adafruit_SSD1306 *oled;
uEEPROMLib *eeprom;
bool timeSyncOK = false;
uint16_t logStartPointer = 0;
uint16_t logEndPointer = 0;

#define LOG_LENGTH_BYTES 128
#define DISPLAY_I2C_ADDRESS 0x3C
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32

void displayTime()
{
    char text[16];
    char textB[6];

    rtc->refresh();

    oled->clearDisplay();
    oled->setTextSize(2);
    oled->setTextColor(SSD1306_WHITE); // Draw white text
    oled->cp437(true);                 // Use full 256 char 'Code Page 437' font

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
    oled->setCursor(100, 25);
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
    eeprom->eeprom_write(logEndPointer, temperatureEncoded);

    logEndPointer = (logEndPointer + 1) % LOG_LENGTH_BYTES;
}

void plotTemperature()
{
    oled->clearDisplay();
    oled->drawLine(4, SCREEN_HEIGHT - 4, SCREEN_WIDTH - 4, SCREEN_HEIGHT - 4, SSD1306_WHITE);
    oled->drawLine(4, SCREEN_HEIGHT - 4, 4, 10, SSD1306_WHITE);

    for (int ix = 0; ix < LOG_LENGTH_BYTES - 8; ix++)
    {
        int dataPoint = (logStartPointer + ix) % LOG_LENGTH_BYTES;
        int8_t temperaturePointA = (eeprom->eeprom_read(dataPoint) - 127) / 2;
        int8_t temperaturePointB = (eeprom->eeprom_read(dataPoint + 1) - 127) / 2;
        oled->drawLine(ix + 4, 15 + SCREEN_HEIGHT - temperaturePointA, ix + 5, 15 + SCREEN_HEIGHT - temperaturePointB, SSD1306_WHITE);
    }

    oled->display();
}

void setup()
{
    Wire.begin();
    Serial.begin(9600);

    rtc = new uRTCLib(0x68);

    eeprom = new uEEPROMLib(0x57);

    dcServices = new DCServices(DC_RADIO_NRF24_V2, rtc);

    oled = new Adafruit_SSD1306(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
    oled->begin(SSD1306_SWITCHCAPVCC, DISPLAY_I2C_ADDRESS);

    oled->clearDisplay();
    oled->display();

    timeSyncOK = dcServices->syncRTCToTimeBroadcast();
}

void loop()
{
    if ((millis() / 1000) % 20 < 10)
    {
        displayTime();
    }
    else
    {
        plotTemperature();
    }

    recordData();

    dcServices->loop();
}
