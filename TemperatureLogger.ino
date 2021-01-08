//
//  A palm shaped lamp showing the current time in COlour coded Roman Numerals.
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
#include <SSD1306AsciiAvrI2c.h>
#include <Sodaq_SHT2x.h>

uRTCLib *rtc;
DCServices *dcServices;
SSD1306AsciiAvrI2c *oled;

#define DISPLAY_I2C_ADDRESS 0x3C

void setup()
{
    Wire.begin();
    Serial.begin(9600);

    rtc = new uRTCLib(0x68);
    dcServices = new DCServices(DC_RADIO_NRF24_V2, rtc);

    oled = new SSD1306AsciiAvrI2c();
    oled->begin(&Adafruit128x64, DISPLAY_I2C_ADDRESS);
    oled->setFont(System5x7);
    oled->set1X();
    oled->clear();
}

void loop()
{
    char text[16];

    rtc->refresh();

    if (rtc->second() % 10 == 0)
    {
        bool syncOK = dcServices->syncRTCToTimeBroadcast();
        oled->setCursor(75, 1);
        oled->print(syncOK ? "*" : " ");
    }

    sprintf(text, "%02i:%02i:%02i", rtc->hour(), rtc->minute(), rtc->second());
    oled->setCursor(80, 1);
    oled->print(text);

    float temperature = SHT2x.GetTemperature() - 1;
    oled->setCursor(0, 2);
    oled->print(temperature);
    oled->print(" C");

    float humidity = SHT2x.GetHumidity();
    oled->setCursor(0, 3);
    oled->print(humidity);
    oled->print(" %");

    dcServices->loop();

    delay(1000);
}
