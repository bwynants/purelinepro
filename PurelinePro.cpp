#include "PurelinePro.h"
#ifdef USE_ESP32
#include "ExtractorHood.h"

namespace esphome
{
  namespace purelinepro
  {

    /******************************* BS444 Scale *******************************************/
    /**
     * Scan for BLE servers and find the first one that advertises the service we are looking for.
     *
     */

    /*
     * The numers in use on the scale
     */
    const uint8_t kSleepTimeBetweenScans = 15;
    const char *TAG = "PurelinePro";

    BLEUUID uartServiceUUID("6e400001-b5a3-f393-e0a9-e50e24dcca9e");
    BLEUUID txCharUUID("6e400003-b5a3-f393-e0a9-e50e24dcca9e");
    BLEUUID rxCharUUID("6e400002-b5a3-f393-e0a9-e50e24dcca9e");

    // we found a device
    void PurelinePro::onResult(BLEAdvertisedDevice &advertisedDevice)
    {
      // ESP_LOGI(TAG, "BLE Device found  %s ", advertisedDevice.toString().c_str());
      //  We have found a device, let us now see if it contains the service we are looking for.
      if (advertisedDevice.haveServiceUUID() && advertisedDevice.isAdvertisingService(Serv_ExtractorHood))
      {
        // devices.insert(advertisedDevice.getAddress());
        ESP_LOGD(TAG, "BLE Advertised Device found  %s", advertisedDevice.toString().c_str());
        BLEDevice::getScan()->stop(); // we found our device
        extractorDevice = new BLEAdvertisedDevice(advertisedDevice);
        mDoConnect = true;
        mDoScan = true;
      }
    }

    void PurelinePro::setup()
    {
      if (fan_)
      {
        fan_->add_on_state_callback([this]()
                                    { ESP_LOGI(TAG, "add_on_state_callback fan_ get_state  %d, speed %d", fan_->state, fan_->speed); });
      }

      if (light_)
      {
        light_->add_on_state_callback([this]()
                                      { ESP_LOGI(TAG, "add_on_state_callback light_ state  %d, brigthness %d, colortemp %d", light_->state_, light_->raw_brightness_, light_->raw_temp_); });
      }

      BLEDevice::init("");

      // Retrieve a Scanner and set the callback we want to use to be informed when we
      // have detected a new device.  Specify that we want active scanning and start the
      // scan to run for 5 seconds.
      BLEScan *pBLEScan = BLEDevice::getScan();

      // Register a callback function to be called when the device is discovered
      //  and send the device to our PurelinePro object
      class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks
      {
      public:
        MyAdvertisedDeviceCallbacks(PurelinePro *purelinePro) : mPurelinePro(purelinePro) {};
        MyAdvertisedDeviceCallbacks() = delete;
        /**
         * Called for each advertising BLE server.
         */
        void onResult(BLEAdvertisedDevice advertisedDevice)
        {
          mPurelinePro->onResult(advertisedDevice);
        } // onResult

      private:
        PurelinePro *mPurelinePro;
      };

      pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks(this));
      pBLEScan->setInterval(1349);
      pBLEScan->setWindow(449);
      pBLEScan->setActiveScan(false);
      mDoConnect = false;
      mDoScan = true;
    }

    void PurelinePro::loop()
    {
      if (millis() > (bletime + 1000))
      {
        bletime = millis();
        // executes every second
        bleClient_loop();
      }
      if (mConnected && packet_ && !statuspending_ && !cmdpending_)
      {
        ESP_LOGD(TAG, "Connected and initialized");
#ifdef USE_SWITCH
        // Power
        if (powertoggle != this->powertoggle_switch_->state)
        {
          ESP_LOGD(TAG, "setting powertoggle %d -> %d", powertoggle, this->powertoggle_switch_->state);
          powertoggle = this->powertoggle_switch_->state;

          std::string msg = "[10;0]";
          rxChar->writeValue(msg);
          ESP_LOGI("UART", "Sent: %s", msg.c_str());
          cmdpending_ = true;
        }
#endif

        if (fanstate_ != (fan_->state ? 1 : 0))
        {
          ESP_LOGD(TAG, "setting fan %d -> %d", fanstate_, fan_->state);
          fanstate_ = (fan_->state ? 1 : 0);

          std::string msg = "[29;1;" + std::string(fanstate_ ? "1" : "0") + "]";
          rxChar->writeValue(msg);
          ESP_LOGI("UART", "Sent: %s", msg.c_str());
          cmdpending_ = true;
        }
        if (fanspeed_ != fan_->speed)
        {
          ESP_LOGI(TAG, "setting fanspeed %d -> %d", fanspeed_, fan_->speed);
          fanspeed_ = fan_->speed;

          std::string msg = "[28;1;" + std::to_string(fanspeed_) + "]";
          rxChar->writeValue(msg);
          ESP_LOGI("UART", "Sent: %s", msg.c_str());
          cmdpending_ = true;
        }

        // Light
        if (lightstate_ != this->light_->state_)
        {
          ESP_LOGD(TAG, "setting power %d -> %d", lightstate_, this->light_->state_);
          lightstate_ = this->light_->state_;

          // 36;0 is off
          // both 15 (ambi) and 16 (white) are lights
          std::string msg = "[" + std::string(lightstate_ ? "16" : "36") + ";0]";
          rxChar->writeValue(msg);
          ESP_LOGI("UART", "Sent: %s", msg.c_str());
          cmdpending_ = true;
        }
        // Bright
        if (lightbrigthness_ != this->light_->raw_brightness_)
        {
          ESP_LOGD(TAG, "setting lightbrigthness %d -> %d", lightbrigthness_, this->light_->raw_brightness_);
          lightbrigthness_ = this->light_->raw_brightness_;

          std::string msg = "[21;1;" + std::to_string(lightbrigthness_) + "]";
          rxChar->writeValue(msg);
          ESP_LOGI("UART", "Sent: %s", msg.c_str());
          cmdpending_ = true;
        }
        // Color
        if (lighttemp_ != this->light_->raw_temp_)
        {
          ESP_LOGD(TAG, "setting lighttemp %d -> %d", lighttemp_, this->light_->raw_temp_);
          lighttemp_ = this->light_->raw_temp_;
          std::string msg = "[22;1;" + std::to_string(lighttemp_) + "]";
          rxChar->writeValue(msg);
          ESP_LOGI("UART", "Sent: %s", msg.c_str());
          cmdpending_ = true;
        } // printServices(pClient);
      }
    }

    void PurelinePro::notifyCallback(BLERemoteCharacteristic *pBLERemoteCharacteristic, uint8_t *pData, size_t length, bool isNotify)
    {
      ESP_LOGD(TAG, "%s callback for characteristic %s, handle %d of data length %d data:", isNotify ? "Notify" : "", pBLERemoteCharacteristic->getUUID().toString().c_str(), pBLERemoteCharacteristic->getHandle(), length);
      if (length && (pData[0] == '[') & (pData[length - 1] == ']'))
      {
        std::string receivedData((char *)pData, length);
        ESP_LOGI(TAG, "Received Reply: %s", receivedData.c_str());
        cmdpending_ = false;
      }
      else if (length == 16)
      {
        esphome::purelinepro::Packet *pkt = reinterpret_cast<Packet *>(pData);
        if ((packet_ == nullptr) || (*packet_ != *pkt))
        {
          if (!packet_)
            packet_ = new Packet();

          if (this->fan_)
          {
            bool publish = false;
            if (pkt->fanspeed != fanspeed_)
            {
              publish = true;
              fanspeed_ = pkt->fanspeed;
              fan_->speed = pkt->fanspeed; // give to HA, no changing to extractor
            }
            int pktState = (pkt->fanspeed > 0) ? 1:0;
            if (pktState != fanstate_)
            {
              publish = true;
              fanstate_ = pktState;
              fan_->state = pktState; // give to HA, no changing to extractor
            }

            if (publish)
              fan_->publish_state();
          }

          if (this->light_)
          {
            bool publish = false;
            if (lightstate_ != pkt->lightmode)
            {
              publish = true;
              lightstate_ = (pkt->lightmode != 0);
            }
            if (lightbrigthness_ != pkt->brightness)
            {
              publish = true;
              lightbrigthness_ = pkt->brightness;
            }
            if (lighttemp_ != pkt->lightmode)
            {
              publish = true;
              lighttemp_ = pkt->colortemp;
            }
            if (publish)
              light_->set_raw(lightstate_ > 0, lightbrigthness_, lighttemp_);
          }

          if (this->timer_sensor_)
          {
            if (this->timer_sensor_->state != pkt->countDown)
              this->timer_sensor_->publish_state(pkt->countDown);
          }

          ESP_LOGI(TAG, "flags: %d%,%d%,%d%,%d%,%d%,%d%,%d%,%d%", pkt->flag0, pkt->timerflag, pkt->flag2, pkt->flag3, pkt->flag4, pkt->flag5, pkt->flag6, pkt->flag7);
          ESP_LOGI(TAG, "speed: %d%", pkt->fanspeed);
          std::string lightMode[] = {"off", "white", "ambi"};
          ESP_LOGI(TAG, "lightmode: %s, b:%d t:%d", lightMode[pkt->lightmode].c_str(), pkt->brightness, pkt->colortemp);
          if (pkt->timerflag && pkt->countDown)
            ESP_LOGI(TAG, "countDown: %d", pkt->countDown);

          if (pkt->unknown1 != 0x00)
            ESP_LOGI(TAG, "unknown1 0x%04X (%d)", pkt->unknown1, (int16_t)pkt->unknown1);
          if (pkt->unknown2 != 0xFF)
            ESP_LOGI(TAG, "unknown2 0x%02X (%d)", pkt->unknown2, (int8_t)pkt->unknown2);
          if (pkt->unknown4 != 0x0100)
            ESP_LOGI(TAG, "unknown4 0x%04X (%d)", pkt->unknown4, (int16_t)pkt->unknown4);
          if (pkt->unknown5 != 0x00FF)
            ESP_LOGI(TAG, "unknown5 0x%04X (%d)", pkt->unknown5, (int16_t)pkt->unknown5);
          if (pkt->unknown6 != 0x0000)
            ESP_LOGI(TAG, "unknown6 0x%04X (%d)", pkt->unknown6, (int16_t)pkt->unknown6);

          *packet_ = *pkt;
        }
        statuspending_ = false;
      }
      else
      {
        parseAndPrintFields(pData, length);
        std::string receivedData((char *)pData, length);
        ESP_LOGI(TAG, "Received Notification: %s", receivedData.c_str());
      }
    }

    void PurelinePro::onDisconnect(BLEClient *pclient)
    {
      ESP_LOGD(TAG, "onDisconnect");
      mConnected = false;
      mDoScan = true;
      rxChar = nullptr;
    }

    // Function that attempts to connect to the device, here is the KEY!!!
    bool PurelinePro::connectToServer()
    {
      // extractorDevice is a variable containing information about the device to be connected.
      // In the device scan below, information will be entered into the corresponding variable.
      ESP_LOGD(TAG, "Creating a connection to %s", extractorDevice->getAddress().toString().c_str());

      // Create a client (Central) class to connect to the server (Pheriphral)
      pClient = BLEDevice::createClient();
      ESP_LOGD(TAG, "Created client");

      class MyClientCallback : public BLEClientCallbacks
      {
      public:
        MyClientCallback(PurelinePro *purelinePro) : mPurelinePro(purelinePro) {};
        MyClientCallback() = delete;

        void onConnect(BLEClient *pclient)
        {
        }

        void onDisconnect(BLEClient *pclient)
        {
          mPurelinePro->onDisconnect(pclient);
        }

      private:
        PurelinePro *mPurelinePro;
      };

      // Set up a callback function to receive connection status events
      pClient->setClientCallbacks(new MyClientCallback(this));
      ESP_LOGD(TAG, "callbacks set");

      // Finally tried to connect to the server (Pheriphral) device!!!
      pClient->connect(extractorDevice); // if you pass BLEAdvertisedDevice instead of address, it will be recognized type of peer device address (public or private)
      ESP_LOGI(TAG, "Connected to server");
      // set client to request maximum MTU from server (default is 23 otherwise)
      pClient->setMTU(517);

      auto nusService = pClient->getService(uartServiceUUID);
      if (nusService)
      {
        auto txChar = nusService->getCharacteristic(txCharUUID);
        if (txChar && txChar->canNotify())
          txChar->registerForNotify(std::bind(&PurelinePro::notifyCallback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
        rxChar = nusService->getCharacteristic(rxCharUUID);
      }
      printServices(pClient);
      // If you have reached this point, set a variable to indicate that the connection was successful.
      mConnected = true;
      return true;
    }

    void PurelinePro::bleClient_loop()
    {
      // If the flag "mDoConnect" is true then we have scanned for and found the desired
      // BLE Server with which we wish to connect.  Now we connect to it.  Once we are
      // connected we set the mConnected flag to be true.
      if (mDoConnect)
      {
        if (connectToServer())
          ESP_LOGI(TAG, "We are now connected to the BLE Server.");
        else
          ESP_LOGE(TAG, "We have failed to connect to the server; there is nothing more we will do.");
        mDoConnect = false;
      }

      if (mConnected)
      {
        // If we are connected to a peer BLE Server, update the characteristic each time we are reached
        // with the current time since boot.
        if (!cmdpending_ && !statuspending_ && rxChar && rxChar->canWrite())
        {
          std::string msg = "[400;0]";
          rxChar->writeValue(msg);
          // ESP_LOGI("UART", "Sent: %s", msg.c_str());
          statuspending_ = true;
        }

        bletime = millis() + 1 * 1000; // next itertation in 1 seconds
      }
      else
      {
        if (mDoScan)
        {
          ESP_LOGI(TAG, "Starting scan in %d seconds.", kSleepTimeBetweenScans);
          // start a next scan in 30 time
          mDoScan = false;
          mScan = true;

          bletime = millis() + kSleepTimeBetweenScans * 1000; // delay next itertation
        }
        if (mScan)
        {
          mScan = false;
          mDoScan = true; // restarting a delayed scan whenever possible
                          // If the connection is released and mDoScan is true, start scanning
          ESP_LOGI(TAG, "Restarting scan.");
          BLEDevice::getScan()->start(1, false);
        }
      }
    }

    void PurelinePro::dump_config()
    {
      ESP_LOGCONFIG(TAG, "PurelinePro:");
      // LOG_FAN("  ", "", this->fan_);
#ifdef USE_SWITCH
      LOG_SWITCH("  ", "PowerToggle", this->powertoggle_switch_);
#endif
      if (this->timer_sensor_)
        LOG_SENSOR(TAG, " timer", this->timer_sensor_);
    }

#ifdef USE_TIME
    void PurelinePro::set_time_id(time::RealTimeClock *time_id)
    {
      ESPTime now = time_id->now();
      ESP_LOGI(TAG, "setting time %ld!", time_id->now().timestamp);
      this->time_id_ = time_id;
    }
#endif

    time_t PurelinePro::now()
    {
#ifdef USE_TIME
      if (this->time_id_.has_value())
      {
        auto *time_id = *this->time_id_;
        ESPTime now = time_id->now();
        return time_id->now().timestamp;
      }
      else
#endif
      {
        ESP_LOGI(TAG, "Time unknown!");
        return millis() / 1000; // some stupid value.....
      }
    }

  } // namespace purelinepro
} // namespace esphome

#endif // USE_ESP32
