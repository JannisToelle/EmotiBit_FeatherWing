#pragma once

#include <SPI.h>
#include <ArduinoBLE.h>
#include "FS.h"
#include "BluetoothPacket.h"

#define BLE_MIN_UPDATE_INTERVAL 100
#define BLE_MAX_UPDATE_INTERVAL 10000
#define DESCRIPTOR_UUID "00002901-0000-1000-8000-00805f9b34fb"
#define BLE_NOT_RECORDING_STAUTS "0"
#define BLE_DATA_TRANSFER_ACTIVE "1"
#define BLE_DATA_TRANSFER_INACTIVE "0"
#define BLE_FILE_TRANSFER_PACKET_SIZE 20
#define MAX_BLE_DATA_LENGTH 512

class EmotibitBluetooth
{
private:
    uint8_t batteryPercentage;
    uint8_t heartRate;
    uint32_t maxPacketSize;
    int calculatePacketCount(int dataLength);
    void sendPacketCountControlPacket(int packetCount);
    void sendDataTransferCompletePacket();
    void sendPackets(const String &data);
    bool dataTransferCancelReceived();

public:
    void setup(const String &deviceName, const String &pairingCode);
    void initServices();
    void sendData();
    int retrieveData(BluetoothPacket* packetType, uint8_t* buffer);
    void updateBatteryLevel(float batteryLevel);
    void updateHeartRate(float heartRate);

    // control functions
    void setUpdateInterval(uint32_t interval);
    void setRecordingSince(const String &recordingSince);

    void transferFile(File &file);
    void transferData(const String &data);
};