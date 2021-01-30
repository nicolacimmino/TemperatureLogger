
#ifndef __TEMPERATURE_LOGGER_GRAPH_DISPLAY_H__
#define __TEMPERATURE_LOGGER_GRAPH_DISPLAY_H__

#include "Display.h"
#include "DataStore.h"

class GraphDisplay : public Display
{
private:
    bool plotAutoscale = false;

    uint8_t valueToYOffset(float value);
    uint8_t plotIndexToXOffset(uint8_t ix);
    void plot();
    uint16_t getStoredValue(uint8_t ix);

protected:
    int8_t minVal = 15;
    int8_t maxVal = 30;

    virtual void setDefaultRange() = 0;
    virtual float getValueFromRawRecord(uint8_t record) = 0;
    virtual uint8_t getRawRecordOffset() = 0;
    virtual float getVTick() = 0;    
    virtual void printPlotTitle() = 0;

public:
    void loop();
    void onBClick();
    void onBLongPress();
};

#endif