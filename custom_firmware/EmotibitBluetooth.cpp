#include "EmotibitBluetooth.h"

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

BLEStringCharacteristic recordingChar("b6c8e04a-693e-4b6b-a068-d1729bd20105", BLERead | BLEWrite, MAX_BLE_DATA_LENGTH);
BLEDescriptor recordingDescr(DESCRIPTOR_UUID, "Recording since");

BLEUnsignedIntCharacteristic runTimeChar("23b55464-c23a-4e6b-a85f-87067e39055d", BLERead);
BLEDescriptor runTimeDescr(DESCRIPTOR_UUID, "Runtime in ms");

BLEStringCharacteristic fileListChar("d4514e95-d053-4a47-b437-6a31f134f84b", BLERead | BLEWrite | BLENotify, MAX_BLE_DATA_LENGTH);
BLEDescriptor fileListDescr(DESCRIPTOR_UUID, "List files");

BLEStringCharacteristic fileTransferChar("74759e9c-ee42-4f67-ad82-d0550baf3ebe", BLERead | BLEWrite | BLENotify, MAX_BLE_DATA_LENGTH);
BLEDescriptor fileTransferDescr(DESCRIPTOR_UUID, "File transfer");

BLEStringCharacteristic fileDeleteChar("daf1816c-2e6e-4d34-b823-bbfbd14715be", BLERead | BLEWrite | BLENotify, MAX_BLE_DATA_LENGTH);
BLEDescriptor fileDeleteDescr(DESCRIPTOR_UUID, "Delete file");

BLEStringCharacteristic dataTransferChar("62df6e1c-cd7f-491d-aa63-f064a18c0ba4", BLERead | BLEWrite | BLENotify, MAX_BLE_DATA_LENGTH);
BLEDescriptor dataTransferDescr(DESCRIPTOR_UUID, "Data transfer");

// Use this to test the maximum mtu. When reading you will receive a packet of the maximum length both devices support
BLEStringCharacteristic mtuTestChar("b6780fc6-0dba-47f1-b5f1-de1e17c1c94c", BLEWrite | BLENotify, MAX_BLE_DATA_LENGTH);
BLEDescriptor mtuTestDescr(DESCRIPTOR_UUID, "Test mtu");

// Use this to set the mtu for all future packets. When set too high you will lose data, so test it first using the mtuTest characteristic
BLEUnsignedIntCharacteristic mtuChar("df1a1a91-c61b-4845-9bd2-9aae919540be", BLERead | BLEWrite);
BLEDescriptor mtuDescr(DESCRIPTOR_UUID, "MTU");

long previousMillis = 0; // last time data was sent over ble
bool wasConnected = false;
uint64_t bleUpdateInterval = 3000;

void EmotibitBluetooth::setup(const String &deviceName, const String &pairingCode)
{
    packetSize = BLE_FILE_TRANSFER_PACKET_SIZE;
    // ignore pairing code for now

    // begin initialization
    if (!BLE.begin())
    {
        Serial.println("starting BLE failed!");
    }

    BLE.setDeviceName(deviceName.c_str());
    BLE.setLocalName(deviceName.c_str());
    BLE.setAdvertisedService(heartRateService);

    BLE.setConnectionInterval(0x0006, 0x0c80); // 7.5 ms minimum, 4 s maximum

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

    // custom services
    updateIntervalChar.addDescriptor(updateIntervalDescr);
    recordingChar.addDescriptor(recordingDescr);
    runTimeChar.addDescriptor(runTimeDescr);
    fileListChar.addDescriptor(fileListDescr);
    fileTransferChar.addDescriptor(fileTransferDescr);
    fileDeleteChar.addDescriptor(fileDeleteDescr);
    dataTransferChar.addDescriptor(dataTransferDescr);
    mtuTestChar.addDescriptor(mtuTestDescr);
    mtuChar.addDescriptor(mtuDescr);

    controlService.addCharacteristic(updateIntervalChar);
    controlService.addCharacteristic(recordingChar);
    controlService.addCharacteristic(runTimeChar);
    controlService.addCharacteristic(fileListChar);
    controlService.addCharacteristic(fileTransferChar);
    controlService.addCharacteristic(fileDeleteChar);
    controlService.addCharacteristic(dataTransferChar);
    controlService.addCharacteristic(mtuTestChar);
    controlService.addCharacteristic(mtuChar);
    BLE.addService(controlService);

    // init characteristic values
    updateIntervalChar.writeValue(bleUpdateInterval);
    recordingChar.writeValue(BLE_NOT_RECORDING_STAUTS);
    runTimeChar.writeValue(millis());
    fileListChar.writeValue(BLE_DATA_TRANSFER_INACTIVE);
    fileTransferChar.writeValue(BLE_DATA_TRANSFER_INACTIVE);
    fileDeleteChar.writeValue(BLE_DATA_TRANSFER_INACTIVE);
    dataTransferChar.writeValue(BLE_DATA_TRANSFER_INACTIVE);
    mtuTestChar.writeValue(std::string(MAX_BLE_DATA_LENGTH, '1').c_str());
    mtuChar.writeValue(packetSize);
}

void EmotibitBluetooth::updateBatteryLevel(float batteryLevel)
{
    batteryPercentage = (uint8_t)batteryLevel;
}

void EmotibitBluetooth::updateHeartRate(float heartRate)
{
    heartRate = (uint8_t)heartRate;
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

            batteryLevelChar.writeValue(batteryPercentage);
            heartRateChar.writeValue(heartRate);
            runTimeChar.writeValue(currentMillis);
        }
    }
    else if (wasConnected)
    {
        wasConnected = false;
        Serial.print("Disconnected from central: ");
        Serial.println(central.address());
    }
}

void EmotibitBluetooth::processEvents()
{
    if (mtuChar.written())
    {
        uint32_t mtu;
        mtuChar.readValue(mtu);
        packetSize = mtu;
        Serial.printf("MTU updated to: %u\n", mtu);
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

void EmotibitBluetooth::setRecordingSince(const String &recordingSince)
{
    recordingChar.writeValue(recordingSince);
}

int EmotibitBluetooth::retrieveData(BluetoothPacket *packetType, uint8_t *buffer)
{
    if (updateIntervalChar.written())
    {
        uint32_t readValue;
        int dataLength = updateIntervalChar.readValue(readValue);

        memcpy(buffer, &readValue, dataLength);
        *packetType = BluetoothPacket::UPDATE_INTERVAL;
        return dataLength;
    }

    if (recordingChar.written())
    {
        int dataLength = recordingChar.valueLength();
        int bytesRead = recordingChar.readValue(buffer, dataLength);

        if (bytesRead != dataLength)
        {
            Serial.printf("Only read %u out of %u bytes", bytesRead, dataLength);
        }
        // add null terminator
        buffer[bytesRead] = 0;
        *packetType = BluetoothPacket::RECORDING;
        return bytesRead + 1;
    }

    if (fileListChar.written())
    {
        fileListChar.readValue(buffer, 1);

        *packetType = BluetoothPacket::FILE_LIST;
        return 1;
    }

    if (fileTransferChar.written())
    {
        int dataLength = fileTransferChar.valueLength();
        int bytesRead = fileTransferChar.readValue(buffer, dataLength);

        if (bytesRead != dataLength)
        {
            Serial.printf("Only read %u out of %u bytes", bytesRead, dataLength);
        }
        // add null terminator
        buffer[bytesRead] = 0;

        *packetType = BluetoothPacket::FILE_TRANSFER;
        return bytesRead + 1;
    }

    if (fileDeleteChar.written())
    {
        int dataLength = fileDeleteChar.valueLength();
        int bytesRead = fileDeleteChar.readValue(buffer, dataLength);

        if (bytesRead != dataLength)
        {
            Serial.printf("Only read %u out of %u bytes", bytesRead, dataLength);
        }
        // add null terminator
        buffer[bytesRead] = 0;

        *packetType = BluetoothPacket::FILE_DELETE;
        return bytesRead + 1;
    }

    return 0;
}

void EmotibitBluetooth::transferFile(File &file)
{
    uint32_t fileSize = file.size();
    char buffer[packetSize + 1];
    int packetCount = calculatePacketCount(fileSize);

    sendPacketCountControlPacket(packetCount);
    for (int i = 0; i < packetCount; i++)
    {
        memset(buffer, 0, (packetSize + 1) * sizeof(char));
        if (dataTransferCancelReceived())
        {
            Serial.println("Cancelling data transfer");
            file.close();
            break;
        }
        int filePosition = i * packetSize;
        file.seek(filePosition);
        int readSize = min(packetSize, fileSize - filePosition);
        file.readBytes(buffer, readSize);
        // add null terminator
        buffer[readSize] = 0;
        dataTransferChar.writeValue(buffer);
    }
    sendDataTransferCompletePacket();
    file.close();
}

void EmotibitBluetooth::transferData(const String &fileData)
{
    int packetCount = calculatePacketCount(fileData.length());
    sendPacketCountControlPacket(packetCount);
    sendPackets(fileData);
    sendDataTransferCompletePacket();
}

int EmotibitBluetooth::calculatePacketCount(int dataLength)
{
    return ceil(1.0f * dataLength / packetSize);
}

void EmotibitBluetooth::sendPacketCountControlPacket(int packetCount)
{
    std::string packetCountMessage = "packetCount=" + std::to_string(packetCount);
    dataTransferChar.writeValue(packetCountMessage.c_str());
}

void EmotibitBluetooth::sendDataTransferCompletePacket()
{
    dataTransferChar.writeValue(BLE_DATA_TRANSFER_INACTIVE);
}

void EmotibitBluetooth::sendPackets(const String &data)
{
    int packetCount = calculatePacketCount(data.length());
    for (int packet = 0; packet < packetCount; packet++)
    {
        if (dataTransferCancelReceived())
        {
            Serial.println("Cancelling data transfer");
            break;
        }
        String packetData = data.substring(packet * packetSize,
                                           packet * packetSize + packetSize);
        dataTransferChar.writeValue(packetData);
    }
}

bool EmotibitBluetooth::dataTransferCancelReceived()
{
    if (!BLE.connected())
    {
        return true;
    }
    if (dataTransferChar.written())
    {
        uint8_t value;
        dataTransferChar.readValue(value);
        char stringValue[2];
        stringValue[0] = value;
        // add null terminator
        stringValue[1] = 0;
        return strcmp(stringValue, BLE_DATA_TRANSFER_INACTIVE) == 0;
    }
    return false;
}