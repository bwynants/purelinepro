#pragma once

#define USE_CMDS

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

    const int cmd_power = 10;
    const int cmd_light_on_ambi = 15;
    const int cmd_light_on_white = 16;
    const int cmd_light_brightness = 21;
    const int cmd_light_colortemp = 22;

    const int cmd_reset_grease = 23;
    const int cmd_fan_recirculate = 25;

    const int cmd_fan_speed = 28;
    const int cmd_fan_state = 29;
    const int cmd_light_off = 36;

    const int cmd_fan_default = 41;
    const int cmd_light_default = 42;

    const int cmd_hood_status = 400;
    const int cmd_hood_status402 = 402;
    const int cmd_hood_status403 = 403;
    const int cmd_hood_status404 = 404;

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
#endif
#ifdef USE_FAN
    public:
      void set_fan(esphome::Component *component) { extractor_fan_ = static_cast<esphome::purelinepro::ExtractorFan *>(component); }

    protected:
      esphome::purelinepro::ExtractorFan *extractor_fan_;
#endif
#ifdef USE_SENSOR
      SUB_SENSOR(timer)
      SUB_SENSOR(greasetimer)
      SUB_SENSOR(operating_hours_led)
      SUB_SENSOR(operating_hours_fan)
#endif
#ifdef USE_BINARY_SENSOR
      // fan boost active
      SUB_BINARY_SENSOR(boost)
      // shutting down in progress
      SUB_BINARY_SENSOR(stopping)
#endif
#ifdef USE_BUTTON
      SUB_BUTTON(power)
      SUB_BUTTON(delayedoff)
      SUB_BUTTON(defaultlight)
      SUB_BUTTON(defaultspeed)
      SUB_BUTTON(resetgrease)
#endif
#ifdef USE_SWITCH
      SUB_SWITCH(recirculate)
      SUB_SWITCH(enabled)
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
      void handleSensors(const Packet402 *pkt);
      void handleSensors(const Packet403 *pkt);
      void handleSensors(const Packet404 *pkt);
      void handleSwitch(const Packet402 *pkt);
      void handleLight(const Packet *pkt);
      void handleFan(const Packet *pkt);

      void handleAck(std::string_view ack);
      void handleStatus400(const Packet *pkt);
      void handleStatus402(const Packet402 *pkt);
      void handleStatus403(const Packet403 *pkt);
      void handleStatus404(const Packet404 *pkt);

    public:
      void send_cmd(int command_id, const std::vector<uint8_t> &args, const std::string &msg, bool log = true);
      void send_cmd(std::string cmd, const std::string &msg, bool log = true);

    protected:
      void recieved_answer(uint8_t *data, uint16_t size);

      void request_status_update();
      void request_status40x_update();

    protected:
      uint16_t rx_char_handle_ = 0;
      uint16_t tx_char_handle_ = 0;

      // last packet received
      std::unique_ptr<struct Packet> packet_ = nullptr;
      std::unique_ptr<struct Packet402> packet402_ = nullptr;
      std::unique_ptr<struct Packet403> packet403_ = nullptr;
      std::unique_ptr<struct Packet404> packet404_ = nullptr;

      // for timeout on request
      uint32_t last_request_ = millis();
      // how many cmd's ae outstanding?
      uint32_t pending_request_ = 0;

      uint32_t status_pending_ = 0;
      uint32_t status40x_pending_ = 0;
      // what cmd did we send
      int status40x_cmd = cmd_hood_status402;
      // counter for delaying the 40x cmd's
      uint32_t status40x_count_ = 0;
      uint32_t status40x_delay_ = 0;
      

      // generic action timer
      uint32_t timer_ = millis();

      // auto off
      uint32_t auto_off_timer_ = millis();
      bool auto_off_ = false;
    };

  } // namespace am43
} // namespace esphome

#endif
