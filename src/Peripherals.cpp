#include "Peripherals.h"

uRTCLib *Peripherals::rtc = NULL;
Adafruit_SSD1306 *Peripherals::oled = NULL;
uEEPROMLib *Peripherals::eeprom = NULL;
Button *Peripherals::buttonA = NULL;
Button *Peripherals::buttonB = NULL;
DCServicesLite *Peripherals::dcServices = NULL;

void Peripherals::setup()
{
    Peripherals::buttonA = new Button(PIN_BUTTON_A);
    Peripherals::buttonB = new Button(PIN_BUTTON_B);
    Peripherals::rtc = new uRTCLib(0x68);
    Peripherals::dcServices = new DCServicesLite(DC_RADIO_NRF24_V2, Peripherals::rtc);
    Peripherals::eeprom = new uEEPROMLib(0x57);
    Peripherals::oled = new Adafruit_SSD1306(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
    Peripherals::oled->begin(SSD1306_SWITCHCAPVCC, DISPLAY_I2C_ADDRESS);
    Peripherals::oled->clearDisplay();
    Peripherals::oled->display();
}