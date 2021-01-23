
#include "PowerManager.h"

 unsigned long PowerManager::lastUserInteractionTime = millis();
 uint8_t PowerManager::level = PS_LEVEL_0;
 uint8_t PowerManager::previousLevel = PS_LEVEL_0;
 Adafruit_SSD1306 *PowerManager::oled;
