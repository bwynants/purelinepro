#pragma once

#ifdef USE_ESP32
#include "esphome.h"
#include "BLEDevice.h"
#include <vector>
#include <set>

#ifdef USE_SWITCH
#include "esphome/components/switch/switch.h"
#endif
#include "esphome/core/component.h"

#ifdef USE_TIME
#include "esphome/components/time/real_time_clock.h"
#include "esphome/core/time.h"
#endif
#include "ExtractorHood.h"
#include "light/extractor_light.h"
#include "fan/extractor_fan.h"

/******************************* BS444 Scale *******************************************/
/**
 * Scan for BLE servers and find the first one that advertises the service we are looking for.
 *
 */

namespace esphome
{
  namespace purelinepro
  {
    extern const uint8_t kSleepTimeBetweenScans;
    extern const char *TAG;

    class PurelinePro : public Component
    {

    public:
      // Connection and device discovery state variables
      bool mConnected = false; // we have a valid connection
      bool mDoConnect = false; // we will try to connect
      bool mDoScan = false;    // will schedule a new scan in 30 seconds
      bool mScan = false;      // will do a new scan

      BLEAdvertisedDevice *extractorDevice;
      BLEClient *pClient;
      // timer
      uint32_t bletime = millis() + 1000;

    public:
      PurelinePro() = default;

    public:
      void dump_config() override;

    protected:
      time_t now();

      // we found a device
      void onResult(BLEAdvertisedDevice &advertisedDevice);

      void setup() override;

      void loop() override;

      void notifyCallback(BLERemoteCharacteristic *pBLERemoteCharacteristic, uint8_t *pData, size_t length, bool isNotify);

      void onDisconnect(BLEClient *pclient);

      // Function that attempts to connect to the device, here is the KEY!!!
      bool connectToServer();

      void bleClient_loop();

#ifdef USE_TIME
    public:
      void set_time_id(time::RealTimeClock *time_id);

    protected:
      optional<time::RealTimeClock *> time_id_{};
#endif

    public:
      void set_timer(sensor::Sensor *sensor) { timer_sensor_ = sensor; }

    protected:
      sensor::Sensor *timer_sensor_ = nullptr;

    public:
      void set_light(esphome::Component *component) { light_ = static_cast<esphome::purelinepro::ExtractorLight *>(component); }
      void set_fan(esphome::Component *component) { fan_ = static_cast<esphome::purelinepro::ExtractorFan *>(component); }

    protected:
      esphome::purelinepro::ExtractorLight *light_;
      esphome::purelinepro::ExtractorFan *fan_;

#ifdef USE_SWITCH
      SUB_SWITCH(powertoggle)
#endif
      bool powertoggle = false;

      int fanstate_ = -1;
      int fanspeed_ = -1;

      int lightstate_ = -1;
      int lightbrigthness_ = -1;
      int lighttemp_ = -1;

      struct Packet *packet_ = nullptr; // last packet
      BLERemoteCharacteristic *rxChar = nullptr;

      bool statuspending_ = false;
      bool cmdpending_ = false;
    };
  } // namespace purelinepro
} // namespace esphome

#endif // USE_ESP32
