#include "EmotibitBluetooth.h"

#define BLE_UPDATE_INTERVAL 3000

BLEService heartRateService("180D");
// BLECharacteristic()
BLEUnsignedIntCharacteristic heartRateChar("2A37", BLERead | BLENotify);

BLEService batteryService("180F");
BLEUnsignedCharCharacteristic batteryLevelChar("2A19", BLERead | BLENotify);

long previousMillis = 0; // last time data was sent over ble
bool wasConnected = false;

void EmotibitBluetooth::setup(String deviceName, String pairingCode)
{
    // ignore pairing code for now

    // begin initialization
    if (!BLE.begin())
    {
        Serial.println("starting BLE failed!");
    }

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
    heartRateService.addCharacteristic(heartRateChar);
    BLE.addService(heartRateService);

    batteryService.addCharacteristic(batteryLevelChar);
    BLE.addService(batteryService);
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
        if (!wasConnected)
        {
            wasConnected = true;
            Serial.print("Connected to central: ");
            // print the central's BT address:
            Serial.println(central.address());
        }

        long currentMillis = millis();
        if (currentMillis - previousMillis >= BLE_UPDATE_INTERVAL)
        {
            previousMillis = currentMillis;

            batteryLevelChar.writeValue(batteryBuffer[0]);
            heartRateChar.writeValue(115);

            size_t timestamp = millis();
            Serial.print("timestamp: ");
            Serial.println(timestamp);
            Serial.print("sending battery percentage: ");
            Serial.println(batteryBuffer[0]);
            Serial.print("sending heart rate: ");
            Serial.println(heartRateBuffer[0]);

            // if (batteryBuffer[0] != batteryBuffer[1])
            // {
            //     batteryLevelChar.writeValue(batteryBuffer[0]);
            //     batteryBuffer[1] = batteryBuffer[0];
            // }
            // if (heartRateBuffer[0] != heartRateBuffer[1])
            // {
            //     heartRateChar.writeValue(heartRateBuffer[0]);
            //     heartRateBuffer[1] = heartRateBuffer[0];
            // }
        }
    }
    else if (wasConnected)
    {
        wasConnected = false;
        Serial.print("Disconnected from central: ");
        Serial.println(central.address());
    }
}