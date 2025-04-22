#pragma once

#define USE_CMDS
//#define FULL_VERSION
// #undef USE_SENSOR
// #undef USE_FAN
// #undef USE_LIGHT
// #undef USE_BUTTON
// #undef USE_SWITCH
// #undef USE_BINARY_SENSOR
// #undef USE_TIME

#ifndef USE_ESP32
#define USE_ESP32
#endif

#ifdef USE_ESP32

#include "esphome/core/component.h"
#include "esphome/components/ble_client/ble_client.h"
#include <esp_gattc_api.h>
#include "esphome/components/esp32_ble_tracker/esp32_ble_tracker.h"
#ifdef USE_SENSOR
#include "esphome/components/sensor/sensor.h"
#endif
#ifdef USE_FAN
#include "esphome/components/fan/fan.h"
#include "fan/extractor_fan.h"
#endif
#ifdef USE_LIGHT
#include "esphome/components/light/light_output.h"
#include "light/extractor_light.h"
#endif
#ifdef USE_BUTTON
#include "esphome/components/button/button.h"
#include "button/extractor_button.h"
#endif
#ifdef USE_SWITCH
#include "esphome/components/switch/switch.h"
#include "switch/extractor_switch.h"
#endif
#ifdef USE_TIME
#include "esphome/components/time/real_time_clock.h"
#include "esphome/core/time.h"
#endif
#ifdef USE_BINARY_SENSOR
#include "esphome/components/binary_sensor/binary_sensor.h"
#endif

#include "ExtractorHood.h"

namespace esphome
{
  namespace purelinepro
  {
    extern const uint8_t kSleepTimeBetweenScans;
    extern const char *TAG;

    namespace espbt = esphome::esp32_ble_tracker;

    class PurelinePro : public esphome::ble_client::BLEClientNode, public PollingComponent
    {
    public:
      void setup() override;
      void loop() override;
      void update() override;
      void gattc_event_handler(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if,
                               esp_ble_gattc_cb_param_t *param) override;
      void dump_config() override;
      float get_setup_priority() const override { return setup_priority::DATA; }

    protected:
      uint16_t char_handle_;

    public:
#ifdef USE_LIGHT
      void set_light(esphome::Component *component) { extractor_light_ = static_cast<esphome::purelinepro::ExtractorLight *>(component); }

    protected:
      esphome::purelinepro::ExtractorLight *extractor_light_;
#else
    public:
      void set_light(esphome::Component *component) {}
#endif
#ifdef USE_FAN
    public:
      void set_fan(esphome::Component *component) { extractor_fan_ = static_cast<esphome::purelinepro::ExtractorFan *>(component); }

    protected:
      esphome::purelinepro::ExtractorFan *extractor_fan_;
#else
    public:
      void set_fan(esphome::Component *component) {}
#endif
#ifdef USE_SENSOR
      SUB_SENSOR(timer)
      SUB_SENSOR(greasetimer)
#else
    public:
      void set_timer_sensor(sensor::Sensor *sensor) {}
      void set_greasetimer_sensor(sensor::Sensor *sensor) {}
#endif
#ifdef USE_BINARY_SENSOR
      // fan boost active
      SUB_BINARY_SENSOR(boost)
      // shutting down in progress
      SUB_BINARY_SENSOR(stopping)
#else
    public:
      void set_boost_binary_sensor(binary_sensor::BinarySensor *binary_sensor) {}
      void set_stopping_binary_sensor(binary_sensor::BinarySensor *binary_sensor) {}

#endif
#ifdef USE_BUTTON
      SUB_BUTTON(power)
      SUB_BUTTON(timedoff)
      SUB_BUTTON(defaultlight)
      SUB_BUTTON(defaultspeed)
      SUB_BUTTON(resetgrease)
#else
    public:
      void set_power_button(button::Button *button) {}
      void set_timedoff_button(button::Button *button) {}
      void set_defaultlight_button(button::Button *button) {}
      void set_defaultspeed_button(button::Button *button) {}
      void set_resetgrease_button(button::Button *button) {}
#endif
#ifdef USE_SWITCH
      SUB_SWITCH(recirculate)
      SUB_SWITCH(enabled)
#else
    public:
      void set_recirculate_switch(switch_::Switch *s) {}
      void set_enabled_switch(switch_::Switch *s) {}
#endif

#ifdef USE_TIME
    public:
      void set_time_id(time::RealTimeClock *time_id)
      {
        this->time_id_ = time_id;
      }

    protected:
      time::RealTimeClock *time_id_ = nullptr;
#else
      void set_time_id(time::RealTimeClock *time_id) {}
#endif

    public:
      void handleSensors(const Packet *pkt);
      void handleSensors(const ExtraPacket *pkt);
      void handleSwitch(const ExtraPacket *pkt);
      void handleLight(const Packet *pkt);
      void handleFan(const Packet *pkt);

      void handleAck(std::string_view ack);
      void handleStatus(const Packet *pkt);
      void handleExtraStatus(const ExtraPacket *pkt);

    public:
      void send_cmd(int command_id, const std::vector<uint8_t> &args, const std::string &msg, bool log = true);
      void send_cmd(std::string cmd, const std::string &msg, bool log = true);

    protected:
      void recieved_answer(uint8_t *data, uint16_t size);

      void request_statusupdate();
      void request_extrastatusupdate();

    protected:
      uint16_t rx_char_handle_;
      uint16_t tx_char_handle_;

      // last packet received
      std::unique_ptr<struct Packet> packet_ = nullptr;
      std::unique_ptr<struct ExtraPacket> extraPacket_ = nullptr;

      // for timeout on request
      uint32_t last_request_ = millis();
      // how many cmd's ae outstanding?
      uint32_t pending_request_ = 0;

      // during a status_pending_ home assistant is in charge.....
      // otherwise het hood is in charge
      uint32_t status_pending_ = 0;
      uint32_t extraStatus_pending_ = 0;
      uint32_t extraStatus_count_ = 0;

      // generic action timer
      uint32_t timer_ = millis();

      // auto off
      uint32_t auto_off_timer_ = millis();
      bool auto_off_ = false;
    };

  } // namespace am43
} // namespace esphome

#endif
