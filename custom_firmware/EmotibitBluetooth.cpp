#include "EmotibitBluetooth.h"

#define BLE_UPDATE_INTERVAL 3000

// BLE Battery Service
BLEService batteryService("180F");
BLEUnsignedCharCharacteristic batteryLevelChar("2A19", BLERead | BLENotify);

long previousMillis = 0; // last time data was sent over ble
bool wasConnected = false;
int oldBatteryLevel = 0;

void EmotibitBluetooth::setup()
{

    // begin initialization
    if (!BLE.begin())
    {
        Serial.println("starting BLE failed!");
    }

    /* Set a local name for the Bluetooth® Low Energy device
       This name will appear in advertising packets
       and can be used by remote devices to identify this Bluetooth® Low Energy device
       The name can be changed but maybe be truncated based on space left in advertisement packet
    */
    BLE.setLocalName("BatteryMonitor");
    BLE.setAdvertisedService(batteryService);           // add the service UUID
    batteryService.addCharacteristic(batteryLevelChar); // add the battery level characteristic
    BLE.addService(batteryService);                     // Add the battery service
    // batteryLevelChar.writeValue(0);       // set initial value for this characteristic

    /* Start advertising Bluetooth® Low Energy.  It will start continuously transmitting Bluetooth® Low Energy
       advertising packets and will be visible to remote Bluetooth® Low Energy central devices
       until it receives a new connection */

    // start advertising
    BLE.advertise();

    Serial.println("Bluetooth® device active, waiting for connections...");
}

void EmotibitBluetooth::update()
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
            updateBatteryLevel();
            updateHeartRate();
        }
    }
    else if (wasConnected)
    {
        wasConnected = false;
        Serial.print("Disconnected from central: ");
        Serial.println(central.address());
    }
}

void EmotibitBluetooth::updateBatteryLevel()
{

    float *data;
    uint16_t dataSize;
    uint32_t timestamp;
    dataSize = m_emotibit->getData(EmotiBit::DataType::BATTERY_PERCENT, &data);

    // Serial.println(timestamp);
    Serial.println(dataSize);
    if (dataSize == 1)
    {
        int batteryLevel = data[0];
        if (batteryLevel != oldBatteryLevel)
        {
            Serial.print("Battery Level % is now: ");
            Serial.println(batteryLevel);
            batteryLevelChar.writeValue(batteryLevel);
            oldBatteryLevel = batteryLevel;
        }
    }
}

void EmotibitBluetooth::updateHeartRate()
{
    float *buffer;
    // uint32_t timestamp;
    size_t dataSize = m_emotibit->getData(EmotiBit::DataType::BATTERY_PERCENT, &buffer);

    // Serial.println(timestamp);
    Serial.println(dataSize);
    if (dataSize == 1)
    {
        int batteryLevel = buffer[0];
        if (batteryLevel != oldBatteryLevel)
        {
            Serial.print("Battery Level % is now: ");
            Serial.println(batteryLevel);
            batteryLevelChar.writeValue(batteryLevel);
            oldBatteryLevel = batteryLevel;
        }
    }
}