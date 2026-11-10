#pragma once

#include "../../bootstrap.h"
#include <NimBLEDevice.h>

static constexpr const char* BLE_CONF_DEVICE_NAME = "J1-Prosthesis";

static constexpr const char* BLE_CONF_MAIN_SERVICE_UUID = "12345678-1234-1234-1234-1234567890ab";

static constexpr const char* BLE_CONF_LED_UUID = "12345678-1234-1234-1234-1234567890ac";

static constexpr uint32_t BLE_SCAN_DURATION_MS = 5000;
static constexpr uint32_t BLE_RECONNECT_INTERVAL_MS = 2000;

#define WATCH_BLE_TASK_NAME       "WatchBleConnect"
#define WATCH_BLE_TASK_STACK_SIZE 8192
#define WATCH_BLE_TASK_PRIORITY   1

/* Security must match the server (BleConfig.hpp): bonding, no MITM, LE Secure Connections */
#define BLE_CONF_BONDING  true
#define BLE_CONF_MITM     false
#define BLE_CONF_SECURE   true
#define BLE_CONF_SET_SECURITY_IO_CAPS BLE_HS_IO_NO_INPUT_OUTPUT

#pragma pack(push, 1)
struct LedPayloadBle
{
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t brigthness;
    uint8_t effect;
};
#pragma pack(pop)

enum LedEffectBle : uint8_t
{
    LED_EFFECT_OFF    = 0,
    LED_EFFECT_ON     = 1,
    LED_EFFECT_SOS    = 2,
    LED_EFFECT_POLICY = 3,
    LED_EFFECT_STROBO = 4,
};

class WatchBleClient
{
public:
    struct ProsthesisBatteryReading
    {
        bool known;
        uint8_t percent;
    };

private:
    static constexpr uint16_t BLE_BATTERY_SERVICE_UUID = 0x180F;
    static constexpr uint16_t BLE_BATTERY_LEVEL_UUID = 0x2A19;
    static constexpr uint32_t BLE_BATTERY_FRESH_MS = 30000;

    NimBLEClient* client = nullptr;
    NimBLERemoteCharacteristic* volatile ledChar = nullptr;
    volatile bool batterySubscribed = false;

    bool known = false;
    uint8_t percent = 0;
    uint32_t seenAtMs = 0;
    portMUX_TYPE batteryMux = portMUX_INITIALIZER_UNLOCKED;

    bool connectToDevice(const NimBLEAdvertisedDevice* dev)
    {
        if (!client)
        {
            client =
                NimBLEDevice::createClient();
        }

        if (!client->connect(dev))
        {
            Serial.println("Falha conexão");
            return false;
        }

        if (!client->secureConnection())
        {
            Serial.println("Falha ao criptografar conexão");
            client->disconnect();
            return false;
        }

        auto service =
            client->getService(
                BLE_CONF_MAIN_SERVICE_UUID
            );

        if (!service)
        {
            Serial.println("Service não encontrado");
            client->disconnect();
            return false;
        }

        NimBLERemoteCharacteristic* characteristic =
            service->getCharacteristic(
                BLE_CONF_LED_UUID
            );

        if (!characteristic)
        {
            Serial.println("Characteristic não encontrada");
            client->disconnect();
            return false;
        }

        ledChar = characteristic;

        subscribeProsthesisBattery();

        Serial.println("BLE conectado");

        return true;
    }

    /* Battery is optional: failing here keeps the LED session alive */
    void subscribeProsthesisBattery()
    {
        NimBLERemoteService* service =
            client->getService(NimBLEUUID(BLE_BATTERY_SERVICE_UUID));

        NimBLERemoteCharacteristic* characteristic =
            service ? service->getCharacteristic(NimBLEUUID(BLE_BATTERY_LEVEL_UUID)) : nullptr;

        if (!characteristic || !characteristic->canNotify())
        {
            Serial.println("Battery Level indisponível");
            return;
        }

        const bool subscribed = characteristic->subscribe(
            true,
            [this](NimBLERemoteCharacteristic*, uint8_t* data, size_t length, bool)
            {
                noteProsthesisBattery(std::string(reinterpret_cast<char*>(data), length));
            }
        );

        if (!subscribed)
        {
            Serial.println("Falha ao assinar Battery Level");
            return;
        }

        noteProsthesisBattery(characteristic->readValue());
        batterySubscribed = true;
    }

    void noteProsthesisAdvertisement(const NimBLEAdvertisedDevice* dev)
    {
        if (!dev)
            return;

        noteProsthesisBattery(dev->getServiceData(
            NimBLEUUID((uint16_t)BLE_BATTERY_SERVICE_UUID)
        ));
    }

    void noteProsthesisBattery(const std::string& data)
    {
        if (data.size() == 1)
        {
            const uint8_t level = static_cast<uint8_t>(data[0]);
            if (level > 100)
                return;

            portENTER_CRITICAL(&batteryMux);
            known = true;
            percent = level;
            seenAtMs = millis();
            portEXIT_CRITICAL(&batteryMux);
            return;
        }

        portENTER_CRITICAL(&batteryMux);
        known = false;
        portEXIT_CRITICAL(&batteryMux);
    }

    static void connectionTask(void* param)
    {
        WatchBleClient* self = static_cast<WatchBleClient*>(param);

        while (true)
        {
            if (!self->isConnected())
            {
                self->connect();
            }

            vTaskDelay(pdMS_TO_TICKS(BLE_RECONNECT_INTERVAL_MS));
        }
    }

public:

    void begin()
    {
        if (NimBLEDevice::isInitialized())
            return;

        NimBLEDevice::init("");

        NimBLEDevice::setSecurityAuth(
            BLE_CONF_BONDING,
            BLE_CONF_MITM,
            BLE_CONF_SECURE
        );
        NimBLEDevice::setSecurityIOCap(BLE_CONF_SET_SECURITY_IO_CAPS);
    }

    void startConnectionTask()
    {
        begin();

        xTaskCreate(
            connectionTask,
            WATCH_BLE_TASK_NAME,
            WATCH_BLE_TASK_STACK_SIZE,
            this,
            WATCH_BLE_TASK_PRIORITY,
            nullptr
        );
    }

    bool connect()
    {
        ledChar = nullptr;
        batterySubscribed = false;

        NimBLEScan* scan =
            NimBLEDevice::getScan();

        scan->setActiveScan(true);
        scan->setDuplicateFilter(0);

        NimBLEScanResults results =
            scan->getResults(BLE_SCAN_DURATION_MS);

        bool found = false;
        bool connected = false;

        for (int i = 0; i < results.getCount(); i++)
        {
            const NimBLEAdvertisedDevice* dev =
                results.getDevice(i);

            const bool nameMatch =
                dev->haveName() &&
                (dev->getName() == BLE_CONF_DEVICE_NAME);

            const bool uuidMatch =
                dev->haveServiceUUID() &&
                dev->isAdvertisingService(
                    NimBLEUUID(BLE_CONF_MAIN_SERVICE_UUID)
                );

            if (nameMatch || uuidMatch)
            {
                Serial.printf(
                    "Dispositivo encontrado (%s)\n",
                    nameMatch ? "nome" : "uuid"
                );

                noteProsthesisAdvertisement(dev);
                found = true;
                connected = connectToDevice(dev);
                break;
            }
        }

        if (!found)
        {
            Serial.printf(
                "Dispositivo não encontrado (scan=%d)\n",
                results.getCount()
            );

            for (int i = 0; i < results.getCount(); i++)
            {
                const NimBLEAdvertisedDevice* dev =
                    results.getDevice(i);

                if (dev->haveName())
                {
                    Serial.printf("  visto: %s\n", dev->getName().c_str());
                }
            }
        }

        scan->clearResults();

        return connected;
    }

    bool isConnected()
    {
        return client && client->isConnected() && ledChar;
    }

    ProsthesisBatteryReading prosthesisBattery()
    {
        bool knownCopy;
        uint8_t percentCopy;
        uint32_t seenAtCopy;

        portENTER_CRITICAL(&batteryMux);
        knownCopy = known;
        percentCopy = percent;
        seenAtCopy = seenAtMs;
        portEXIT_CRITICAL(&batteryMux);

        if (!knownCopy)
            return {false, 0};

        /* Notify only fires on change: while subscribed the server owns staleness */
        const bool live = batterySubscribed && isConnected();

        if (!live && millis() - seenAtCopy >= BLE_BATTERY_FRESH_MS)
            return {false, 0};

        return {true, percentCopy};
    }

    bool send(
        const uint8_t r,
        const uint8_t g,
        const uint8_t b,
        const uint8_t brightness,
        const LedEffectBle effect
    )
    {
        NimBLERemoteCharacteristic* characteristic = ledChar;

        if (!characteristic || !isConnected())
            return false;

        LedPayloadBle payload{r, g, b, brightness, effect};

        return characteristic->writeValue(
            (uint8_t*)&payload,
            sizeof(payload),
            true
        );
    }
};
