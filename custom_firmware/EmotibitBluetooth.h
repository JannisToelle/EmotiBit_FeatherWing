#pragma once

#include <SPI.h>
#include <ArduinoBLE.h>

class EmotibitBluetooth
{
private:
    // index 0 holds current value, index 1 hold previous value
    uint8_t batteryBuffer[2];
    uint8_t heartRateBuffer[2];
    void initBuffers();

public:
    void setup(String deviceName, String pairingCode);
    void initServices();
    void sendData();
    void updateBatteryLevel(float batteryLevel);
    void updateHeartRate(float heartRate);
};