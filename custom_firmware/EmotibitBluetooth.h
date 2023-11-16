#pragma once

#include <SPI.h>
#include <ArduinoBLE.h>
#include "EmotiBit.h"

class EmotibitBluetooth
{
private:
    EmotiBit *m_emotibit;

public:
    EmotibitBluetooth(EmotiBit *emotibit)
    {
        m_emotibit = emotibit;
    }

    void setup();
    void update();
    void updateBatteryLevel();
    void updateHeartRate();
};