#pragma once

#include <SPI.h>
#include <ArduinoBLE.h>
#include "BluetoothPacket.h"

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
    bool retrieveData(BluetoothPacket* packetType, uint32_t* buffer);
    void updateBatteryLevel(float batteryLevel);
    void updateHeartRate(float heartRate);

    // control functions
    void setUpdateInterval(uint32_t interval);
    void setRecordingStatus(uint8_t status);
};