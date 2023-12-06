#include "EmotibitBluetooth.h"

#define BLE_MIN_UPDATE_INTERVAL 100
#define BLE_MAX_UPDATE_INTERVAL 10000
#define DESCRIPTOR_UUID "00002901-0000-1000-8000-00805f9b34fb"


BLEService heartRateService("0000180d-0000-1000-8000-00805f9b34fb");
BLEUnsignedIntCharacteristic heartRateChar("00002a37-0000-1000-8000-00805f9b34fb", BLERead | BLENotify);
BLEDescriptor heartRateDescr(DESCRIPTOR_UUID, "Heart rate");

BLEService batteryService("0000180f-0000-1000-8000-00805f9b34fb");
BLEUnsignedIntCharacteristic batteryLevelChar("00002a19-0000-1000-8000-00805f9b34fb", BLERead | BLENotify);
BLEDescriptor batteryLevelDescr(DESCRIPTOR_UUID, "Battery level");

// UUIDs generated with https://www.uuidgenerator.net/
BLEService controlService("84c08996-94a3-45c0-b31c-3504b77ce3ee");
BLEUnsignedIntCharacteristic updateIntervalChar("48949191-cf24-427b-bad6-036e4532e00f", BLERead | BLEWrite | BLENotify);
BLEDescriptor updateIntervalDescr(DESCRIPTOR_UUID, "Bluetooth update interval");
BLEUnsignedIntCharacteristic recordingChar("b6c8e04a-693e-4b6b-a068-d1729bd20105", BLERead | BLEWrite | BLENotify);
BLEDescriptor recordingDescr(DESCRIPTOR_UUID, "Emotibit recording status");

long previousMillis = 0; // last time data was sent over ble
bool wasConnected = false;
uint64_t bleUpdateInterval = 3000;

void EmotibitBluetooth::setup(String deviceName, String pairingCode)
{
    // ignore pairing code for now

    // begin initialization
    if (!BLE.begin())
    {
        Serial.println("starting BLE failed!");
    }

    BLE.setDeviceName(deviceName.c_str());
    BLE.setLocalName(deviceName.c_str());
    BLE.setAdvertisedService(heartRateService);

    initBuffers();
    initServices();

    /* Start advertising Bluetooth® Low Energy.  It will start continuously transmitting Bluetooth® Low Energy
       advertising packets and will be visible to remote Bluetooth® Low Energy central devices
       until it receives a new connection */

    // start advertising
    BLE.advertise();

    Serial.println("Bluetooth® device active, waiting for connections...");
}

void EmotibitBluetooth::initServices()
{
    // add characteristics and register services
    heartRateService.addCharacteristic(heartRateChar);
    heartRateChar.addDescriptor(heartRateDescr);
    BLE.addService(heartRateService);

    batteryService.addCharacteristic(batteryLevelChar);
    batteryLevelChar.addDescriptor(batteryLevelDescr);
    BLE.addService(batteryService);

    updateIntervalChar.addDescriptor(updateIntervalDescr);
    recordingChar.addDescriptor(recordingDescr);
    controlService.addCharacteristic(updateIntervalChar);
    controlService.addCharacteristic(recordingChar);
    BLE.addService(controlService);

    // init characteristic values
    updateIntervalChar.writeValue(bleUpdateInterval);
    recordingChar.writeValue(0);
}

void EmotibitBluetooth::updateBatteryLevel(float batteryLevel)
{
    batteryBuffer[0] = (uint8_t)batteryLevel;
}

void EmotibitBluetooth::updateHeartRate(float heartRate)
{
    heartRateBuffer[0] = (uint8_t)heartRate;
}

void EmotibitBluetooth::initBuffers()
{
    batteryBuffer[0] = 0;
    batteryBuffer[1] = 0;
    heartRateBuffer[0] = 0;
    heartRateBuffer[1] = 0;
}

void EmotibitBluetooth::sendData()
{
    // wait for a BLE central
    BLEDevice central = BLE.central();

    // if a central is connected to the peripheral:
    if (central && central.connected())
    {
        BLE.stopAdvertise();
        if (!wasConnected)
        {
            wasConnected = true;
            Serial.print("Connected to central: ");
            // print the central's BT address:
            Serial.println(central.address());
        }

        long currentMillis = millis();
        if (currentMillis - previousMillis >= bleUpdateInterval)
        {
            previousMillis = currentMillis;

            if (batteryBuffer[0] != batteryBuffer[1])
            {
                batteryLevelChar.writeValue(batteryBuffer[0]);
                batteryBuffer[1] = batteryBuffer[0];
                Serial.print("sending battery percentage: ");
                Serial.println(batteryBuffer[0]);
            }
            if (heartRateBuffer[0] != heartRateBuffer[1])
            {
                heartRateChar.writeValue(heartRateBuffer[0]);
                heartRateBuffer[1] = heartRateBuffer[0];
                Serial.print("sending heart rate: ");
                Serial.println(heartRateBuffer[0]);
            }
        }
    }
    else if (wasConnected)
    {
        wasConnected = false;
        Serial.print("Disconnected from central: ");
        Serial.println(central.address());
    }
}

void EmotibitBluetooth::setUpdateInterval(uint32_t interval)
{
    if (interval < BLE_MIN_UPDATE_INTERVAL)
    {
        bleUpdateInterval = BLE_MIN_UPDATE_INTERVAL;
    }
    else if (interval > BLE_MAX_UPDATE_INTERVAL)
    {
        bleUpdateInterval = BLE_MAX_UPDATE_INTERVAL;
    }
    else
    {
        bleUpdateInterval = interval;
    }
    updateIntervalChar.writeValue(bleUpdateInterval);
}

void EmotibitBluetooth::setRecordingStatus(uint8_t status)
{
    recordingChar.writeValue(status);
}

bool EmotibitBluetooth::retrieveData(BluetoothPacket *packetType, uint32_t *buffer)
{
    if (updateIntervalChar.written())
    {
        uint32_t readValue;
        // int bytesRead = updateIntervalChar.readValue((uint8_t&)readValue);
        int bytesRead = updateIntervalChar.readValue(readValue);

        Serial.println(readValue);
        Serial.println(bytesRead);
        *buffer = readValue;
        *packetType = BluetoothPacket::UPDATE_INTERVAL;

        return true;
    }

    if (recordingChar.written())
    {
        uint8_t readValue = 0;
        int bytesRead = updateIntervalChar.readValue(readValue);

        Serial.println(readValue);
        Serial.println(bytesRead);
        *buffer = readValue;
        *packetType = BluetoothPacket::RECORDING;

        return true;
    }

    return false;
}