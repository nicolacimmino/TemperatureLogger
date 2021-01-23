#include "Peripherals.h"

uRTCLib *Peripherals::rtc = NULL;
Adafruit_SSD1306 *Peripherals::oled = NULL;
uEEPROMLib *Peripherals::eeprom = NULL;
Button *Peripherals::buttonA = NULL;
Button *Peripherals::buttonB = NULL;
