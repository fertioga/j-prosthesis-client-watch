#pragma once

#include "bootstrap.h"
#include <NimBLEDevice.h>

static constexpr const char* BLE_CONF_MAIN_SERVICE_UUID = "12345678-1234-1234-1234-1234567890ab";

static constexpr const char* BLE_CONF_LED_UUID = "12345678-1234-1234-1234-1234567890ac";


struct LedPayloadBle
{
    uint8_t command;
};

class WatchBleClient
{
private:
    NimBLEClient* client = nullptr;
    NimBLERemoteCharacteristic* ledChar = nullptr;

public:

    bool connect()
    {
        NimBLEDevice::init("");

        NimBLEScan* scan =
            NimBLEDevice::getScan();

        scan->setActiveScan(true);

        NimBLEScanResults results =
            scan->getResults(5);

        for (int i = 0; i < results.getCount(); i++)
        {
            const NimBLEAdvertisedDevice* dev =
                results.getDevice(i);

            if (
                dev->haveServiceUUID() &&
                dev->isAdvertisingService(
                    NimBLEUUID(BLE_CONF_MAIN_SERVICE_UUID)
                )
            )
            {
                Serial.println("Dispositivo encontrado");

                client =
                    NimBLEDevice::createClient();

                if (!client->connect(dev))
                {
                    Serial.println("Falha conexão");
                    return false;
                }

                auto service =
                    client->getService(
                        BLE_CONF_MAIN_SERVICE_UUID
                    );

                if (!service)
                {
                    Serial.println("Service não encontrado");
                    return false;
                }

                ledChar =
                    service->getCharacteristic(
                        BLE_CONF_LED_UUID
                    );

                if (!ledChar)
                {
                    Serial.println("Characteristic não encontrada");
                    return false;
                }

                Serial.println("BLE conectado");

                return true;
            }
        }

        Serial.println("Dispositivo não encontrado");

        return false;
    }

    bool send(
        const uint8_t brightness
    )
    {
        if (!ledChar)
            return false;

        LedPayloadBle payload;

        payload.command = 0x01; // Example command, adjust as needed

        return ledChar->writeValue(
            (uint8_t*)&payload,
            sizeof(payload),
            true
        );
    }
};