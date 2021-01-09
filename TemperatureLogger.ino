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
#include <SSD1306AsciiAvrI2c.h>
#include <Sodaq_SHT2x.h>

uRTCLib *rtc;
DCServices *dcServices;
SSD1306AsciiAvrI2c *oled;
uEEPROMLib *eeprom;

#define DISPLAY_I2C_ADDRESS 0x3C

void displayTime()
{
    char text[16];
    char textB[6];

    rtc->refresh();

    oled->set2X();
    sprintf(text, "%02i:%02i:%02i", rtc->hour(), rtc->minute(), rtc->second());
    oled->setCursor(20, 2);
    oled->print(text);

    float temperature = (SHT2x.GetTemperature() * 10) / 10.0;
    float humidity = round(SHT2x.GetHumidity());

    sprintf(text, "%sC", dtostrf(temperature, 3, 1, textB));
    oled->setCursor(0, 6);
    oled->print(text);

    sprintf(text, "%s%%", dtostrf(humidity, 3, 0, textB));
    oled->setCursor(80, 6);
    oled->print(text);
}

void syncTime()
{
    for (uint8_t ix = 0; ix < 6; ix++)
    {
        bool syncOK = dcServices->syncRTCToTimeBroadcast();
        oled->setCursor(0, 1);
        oled->print(ix % 2 == 0 ? "Timesync...." : "                ");

        if (syncOK)
        {
            oled->setCursor(0, 1);
            oled->print("TS                 ");
            return;
        }
    }
}

void setup()
{
    Wire.begin();
    Serial.begin(9600);

    rtc = new uRTCLib(0x68);

    eeprom = new uEEPROMLib(0x57);

    dcServices = new DCServices(DC_RADIO_NRF24_V2, rtc);

    oled = new SSD1306AsciiAvrI2c();
    oled->begin(&Adafruit128x64, DISPLAY_I2C_ADDRESS);
    oled->setFont(System5x7);
    oled->set1X();
    oled->clear();

    syncTime();
}

void loop()
{
    displayTime();

    dcServices->loop();

    delay(500);
}
