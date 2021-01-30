#include "ModeManager.h"

Display *ModeManager::currentDisplay = NULL;
uint8_t ModeManager::mode = 0;

void ModeManager::setup()
{
    mode = 0;
    enterMode();
}

void ModeManager::enterMode()
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
    case 2:
        currentDisplay = new HumidityDisplay();
        break;
    case 3:
        currentDisplay = new StatusDisplay();
        break;
    }
}

void ModeManager::changeMode()
{
    mode = (mode + 1) % DISPLAY_MODES;
    enterMode();
}
