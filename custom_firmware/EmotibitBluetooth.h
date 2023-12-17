#pragma once

#include <SPI.h>
#include <ArduinoBLE.h>
#include "BluetoothPacket.h"

// 100 bytes for data + 1 byte for null terminators in case of strings
#define MAX_BLE_DATA_LENGTH 100 + 1
#define BLE_NOT_RECORDING_STAUTS "0"

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
    int retrieveData(BluetoothPacket* packetType, uint8_t* buffer);
    void updateBatteryLevel(float batteryLevel);
    void updateHeartRate(float heartRate);

    // control functions
    void setUpdateInterval(uint32_t interval);
    void setRecordingSince(String recordingSince);
};