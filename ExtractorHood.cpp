#include "esphome.h"
#ifdef USE_TIME
#include "esphome/core/time.h"
#endif

namespace esphome
{
  namespace purelinepro
  {

    BLEUUID Serv_ExtractorHood("8ec90001-f315-4f60-9fb8-838830daea50"); // the service

    void parseAndPrintFields(const uint8_t *data, size_t len)
    {
      if (len < 16)
      {
        ESP_LOGD(TAG, "Too short\n");
        return;
      }

      for (size_t i = 0; i + 1 < len; i += 2)
      {
        uint16_t val = data[i] | (data[i + 1] << 8);
        ESP_LOGD(TAG, "Field %02zu: 0x%04X (%d)", i / 2, val, (int16_t)val);
      }
    }

    void printServices(BLEAdvertisedDevice *advertisedDevice)
    {
      BLEClient *pClient = BLEDevice::createClient();
      ESP_LOGI(TAG, "Created client");

      class MyClientCallback : public BLEClientCallbacks
      {
      public:
        MyClientCallback() {};

        void onConnect(BLEClient *pclient)
        {
        }

        void onDisconnect(BLEClient *pclient)
        {
        }
      };

      // Set up a callback function to receive connection status events
      pClient->setClientCallbacks(new MyClientCallback());

      if (!pClient->connect(advertisedDevice))
      {
        ESP_LOGE(TAG, "Failed to connect to device");
        return;
      }

      ESP_LOGI(TAG, "Connected to server");

      pClient->setMTU(517); // Request max MTU

      // 🔹 GAP (Generic Access Profile)
      auto gapService = pClient->getService(BLEUUID((uint16_t)0x1800));
      if (gapService)
      {
        auto deviceNameChar = gapService->getCharacteristic(BLEUUID((uint16_t)0x2A00));
        if (deviceNameChar && deviceNameChar->canRead())
        {
          std::string name = deviceNameChar->readValue();
          ESP_LOGI(TAG, "Device Name: %s", name.c_str());
        }
      }

      // 🔹 DIS (Device Information Service)
      auto disService = pClient->getService(BLEUUID((uint16_t)0x180A));
      if (disService)
      {
        auto modelChar = disService->getCharacteristic(BLEUUID((uint16_t)0x2A24));
        auto mfrChar = disService->getCharacteristic(BLEUUID((uint16_t)0x2A29));

        if (modelChar && modelChar->canRead())
        {
          std::string model = modelChar->readValue();
          ESP_LOGI(TAG, "Model Number: %s", model.c_str());
        }
        if (mfrChar && mfrChar->canRead())
        {
          std::string mfr = mfrChar->readValue();
          ESP_LOGI(TAG, "Manufacturer Name: %s", mfr.c_str());
        }
      }

      // 🔹 Nordic UART Service (NUS)
      BLEUUID uartServiceUUID("6e400001-b5a3-f393-e0a9-e50e24dcca9e");
      BLEUUID txCharUUID("6e400003-b5a3-f393-e0a9-e50e24dcca9e");
      BLEUUID rxCharUUID("6e400002-b5a3-f393-e0a9-e50e24dcca9e");

      auto nusService = pClient->getService(uartServiceUUID);
      if (nusService)
      {
        auto txChar = nusService->getCharacteristic(txCharUUID);
        auto rxChar = nusService->getCharacteristic(rxCharUUID);

        if (txChar && txChar->canNotify())
        {
          txChar->registerForNotify([](BLERemoteCharacteristic *c, uint8_t *data, size_t len, bool isNotify)
                                    {
                  std::string received((char *)data, len);
                  ESP_LOGI("UART", "Received: %s", received.c_str()); });
        }

        if (rxChar && rxChar->canWrite())
        {
          std::string msg = "Hello Nordic!";
          rxChar->writeValue(msg);
          ESP_LOGI("UART", "Sent: %s", msg.c_str());
        }
      }

      // 🔹 List all services and characteristics
      auto services = pClient->getServices();
      for (auto &servicePair : *services)
      {
        auto *service = servicePair.second;
        ESP_LOGI(TAG, "Service UUID: %s", service->getUUID().toString().c_str());

        auto characteristics = service->getCharacteristics();
        for (auto &charPair : *characteristics)
        {
          auto *characteristic = charPair.second;

          std::string props;
          if (characteristic->canRead())
            props += "Read ";
          if (characteristic->canWrite())
            props += "Write ";
          if (characteristic->canNotify())
            props += "Notify ";
          if (characteristic->canIndicate())
            props += "Indicate ";

          ESP_LOGI(TAG, "  Characteristic UUID: %s [%s]",
                   characteristic->getUUID().toString().c_str(), props.c_str());
        }
      }
    }
    void printServices(BLEClient *pClient)
    {
      // we do nothing with the connection, we only wait for messages in the callback
      // Receive service information when connected
      for (auto service : *pClient->getServices())
      {
        BLERemoteService *pRemoteService = service.second;
        ESP_LOGI(TAG, "service %s found", pRemoteService->toString().c_str());

        //
        // subscribe to characteristics
        //
        for (auto characteristic : *pRemoteService->getCharacteristics())
        {
          auto pRemoteCharacteristic = characteristic.second;
          if (pRemoteCharacteristic == nullptr)
          {
            continue;
          }

          ESP_LOGI(TAG, "Found characteristic: Handle: %d, UUID: %s", pRemoteCharacteristic->getHandle(), characteristic.first.c_str());
          if (pRemoteCharacteristic->canWrite())
          {
            ESP_LOGI(TAG, "The characteristic canWrite");
          }

          // Read the value of the characteristic.
          if (pRemoteCharacteristic->canRead())
          {
            ESP_LOGI(TAG, "The characteristic canRead");
            std::string value = pRemoteCharacteristic->readValue();
            ESP_LOGI(TAG, "The characteristic value was: %s", value.c_str());
          }

          if (pRemoteCharacteristic->canNotify())
          {
            ESP_LOGI(TAG, "The characteristic canNotify");
            //pRemoteCharacteristic->registerForNotify(std::bind(&PurelinePro::notifyCallback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
            ESP_LOGI(TAG, "Registered Callback");
          }

          if (pRemoteCharacteristic->canIndicate())
          {
            // ESP_LOGI(TAG, "The characteristic canIndicate");
            // pRemoteCharacteristic->registerForNotify(std::bind(&PurelinePro::notifyCallback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
            // ESP_LOGI(TAG, "Registered Callback");

            for (auto descriptor : *pRemoteCharacteristic->getDescriptors())
            {
              ESP_LOGI(TAG, "descriptor:  %s, handle %d", descriptor.first.c_str(), descriptor.second->getHandle());
            }
          }
        }
      }
      ESP_LOGI(TAG, "done");
    }

  } // namespace purelinepro
} // namespace esphome
