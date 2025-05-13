#include <iomanip>
#include "PurelinePro.h"

#ifdef USE_ESP32
#include "esphome/core/log.h"

namespace esphome
{
  namespace purelinepro
  {

    const char *TAG = "PurelinePro";
    const char *UARTTAG = "UART";
    esp32_ble::ESPBTUUID uartServiceUUID = esp32_ble::ESPBTUUID::from_raw("6e400001-b5a3-f393-e0a9-e50e24dcca9e");
    esp32_ble::ESPBTUUID txCharUUID = esp32_ble::ESPBTUUID::from_raw("6e400003-b5a3-f393-e0a9-e50e24dcca9e");
    esp32_ble::ESPBTUUID rxCharUUID = esp32_ble::ESPBTUUID::from_raw("6e400002-b5a3-f393-e0a9-e50e24dcca9e");

    void PurelinePro::handleAck(std::string_view ack)
    {
    }

    void PurelinePro::handleSensors(const Packet *pkt)
    {
      ESP_LOGD(TAG, "handleSensors");
      // we have our own timer and the hood has a timer, combine those in reporting to HA
      // work with the timer....
      uint32_t auto_off_timer = pkt->getBoost() ? 0 : pkt->getTimer();

      if (this->auto_off_ && (auto_off_timer > 0))
      {
        ESP_LOGW(TAG, "auto_off_ stopped, timer of hood active");
        this->auto_off_ = false; // we can not have 2 timers running;
      }

      if (this->auto_off_)
      {
        auto currentTime = millis();
        if (currentTime > this->auto_off_timer_)
        {
          ESP_LOGI(TAG, "auto_off_timer_ done");
          this->auto_off_ = false; // stop running
          auto_off_timer = 0;

          // put fan off
          this->extractor_fan_->state = 0;
          this->extractor_fan_->publish_state();
        }
        else
        {
          // update value
          auto_off_timer = (this->auto_off_timer_ - currentTime) / 1000;
          ESP_LOGD(TAG, "auto_off_timer new value %d", auto_off_timer);
        }
      }
#ifdef USE_SENSOR
      if (this->off_timer_sensor_)
      {
        if (!this->off_timer_sensor_->has_state() || (this->off_timer_sensor_->state != auto_off_timer))
          this->off_timer_sensor_->publish_state(auto_off_timer);
      }
      if (this->boost_timer_sensor_)
      {
        if (!this->boost_timer_sensor_->has_state() || (this->boost_timer_sensor_->state != (pkt->getBoost() ? pkt->getTimer() : 0)))
          this->boost_timer_sensor_->publish_state(pkt->getBoost() ? pkt->getTimer() : 0);
      }
#endif
#ifdef USE_BINARY_SENSOR
      if (this->boost_binary_sensor_)
      {
        if (!this->boost_binary_sensor_->has_state() || (this->boost_binary_sensor_->state != pkt->getBoost()))
          this->boost_binary_sensor_->publish_state(pkt->getBoost());
      }
      if (this->stopping_binary_sensor_)
      {
        auto state = pkt->getStopping() || this->auto_off_;
        if (!this->stopping_binary_sensor_->has_state() || (this->stopping_binary_sensor_->state != state))
          this->stopping_binary_sensor_->publish_state(state);
      }
#endif
    }
    void PurelinePro::handleSensors(const Packet402 *pkt)
    {
      ESP_LOGD(TAG, "handleSensors");
#ifdef USE_SENSOR
      if (this->grease_timer_sensor_)
      {
        if (this->grease_timer_sensor_->state != pkt->getGreaseTimer())
          this->grease_timer_sensor_->publish_state(pkt->getGreaseTimer());
      }
#endif
    }
    void PurelinePro::handleSensors(const Packet403 *pkt)
    {
      ESP_LOGD(TAG, "handleSensors");
#ifdef USE_SENSOR
      if (this->operating_hours_fan_sensor_)
      {
        if (this->operating_hours_fan_sensor_->state != pkt->getFanTimer())
          this->operating_hours_fan_sensor_->publish_state(pkt->getFanTimer());
      }
#endif
    }
    void PurelinePro::handleSensors(const Packet404 *pkt)
    {
      ESP_LOGD(TAG, "handleSensors");
#ifdef USE_SENSOR
      if (this->operating_hours_led_sensor_)
      {
        if (this->operating_hours_led_sensor_->state != pkt->getLedTimer())
          this->operating_hours_led_sensor_->publish_state(pkt->getLedTimer());
      }
#endif
    }
    void PurelinePro::handleSwitch(const Packet402 *pkt)
    {
#ifdef USE_SWITCH
      if (this->recirculate_switch_)
      {
        if (this->recirculate_switch_->state != pkt->getRecirculate())
          this->recirculate_switch_->publish_state(pkt->getRecirculate());
      }
#endif
    }

    void PurelinePro::handleFan(const Packet *pkt)
    {
      ESP_LOGD(TAG, "handleFan");
#ifdef USE_FAN
      if (this->extractor_fan_)
      {
        bool publish = false;
        if (this->extractor_fan_->state != pkt->getFanState())
        {
          ESP_LOGD(TAG, "setting HA fan %d (is %d)", pkt->getFanState(), this->extractor_fan_->state);
          publish = true;
          this->extractor_fan_->state = pkt->getFanState(); // give to HA, no changing to extractor
        }
        if (pkt->getFanState())
        {
          if (this->extractor_fan_->speed != pkt->getFanSpeed())
          {
            publish = true;
            ESP_LOGD(TAG, "setting HA fan speed %d (is %d)", pkt->getFanSpeed(), this->extractor_fan_->speed);
            this->extractor_fan_->speed = pkt->getFanSpeed(); // give to HA, no changing to extractor
          }
        }
        if (publish)
        {
          if (this->auto_off_)
            ESP_LOGI(TAG, "auto_off_timer_ stopped");
          this->auto_off_ = false; // stop running
          this->extractor_fan_->publish_state();
        }
      }
#endif
    }

    void PurelinePro::handleLight(const Packet *pkt)
    {
      ESP_LOGD(TAG, "handleLight");
#ifdef USE_LIGHT
      if (this->extractor_light_)
      {
        bool publish = false;

        if (this->extractor_light_->state_ != pkt->getLightState())
        {
          ESP_LOGD(TAG, "setting HA light %d (is  %d)", packet_->getLightState(), this->extractor_light_->state_);
          this->extractor_light_->state_ = pkt->getLightState();
          publish = true;
        }
        if (pkt->getLightState())
        {
          if (this->extractor_light_->raw_brightness_ != pkt->getBrightness())
          {
            ESP_LOGD(TAG, "setting HA light B %d (is %d)", packet_->getBrightness(), this->extractor_light_->raw_brightness_);
            this->extractor_light_->raw_brightness_ = pkt->getBrightness();
            publish = true;
          }
          if (this->extractor_light_->raw_temp_ != pkt->getColorTemp())
          {
            ESP_LOGD(TAG, "setting HA light T %d (is %d)", packet_->getColorTemp(), this->extractor_light_->raw_temp_);
            this->extractor_light_->raw_temp_ = pkt->getColorTemp();
            publish = true;
          }
        }
        if (publish)
          this->extractor_light_->publish(this->extractor_light_->state_, this->extractor_light_->raw_brightness_, this->extractor_light_->raw_temp_);
      }
#endif
    }

    void PurelinePro::handleStatus400(const Packet *pkt)
    {
      ESP_LOGD(TAG, "handleStatus400");
      if (!pkt)
      {
        ESP_LOGE(TAG, "Invalid packet");
        return;
      }

      if (packet_)
      {
        this->packet_->diff(pkt);
        // settings applied from extractor to HA
        *this->packet_ = *pkt;
      }
      else
      {
        // skip first, let HA restore sates it knows
        this->packet_ = std::make_unique<struct Packet>(*pkt);
      }

      if (this->status_pending_ == 0)
      {
        ESP_LOGD(TAG, "Status received and handling");
        handleFan(pkt);
        handleLight(pkt);
      }
      else
      {
        ESP_LOGW(TAG, "Status skipped %d", this->status_pending_);
      }
      // update sensors always, independend of status_pending_
      handleSensors(this->packet_.get());
    }

    void PurelinePro::handleStatus402(const Packet402 *pkt402)
    {
      ESP_LOGD(TAG, "handleStatus402");
      if (!pkt402)
      {
        ESP_LOGE(TAG, "Invalid 402 packet");
        return;
      }
      if (packet402_)
      {
        this->packet402_->diff(pkt402);
        // settings applied from extractor to HA
        *this->packet402_ = *pkt402;
      }
      else
      {
        // skip first, let HA restore sates it knows
        this->packet402_ = std::make_unique<struct Packet402>(*pkt402);
      }
      if (this->status40x_pending_ == 0)
      {
        ESP_LOGD(TAG, "402 Status received and handling");
        handleSwitch(pkt402);
      }
      else
      {
        ESP_LOGW(TAG, "402 skipped %d", this->status40x_pending_);
      }
      handleSensors(pkt402);
    }

    void PurelinePro::handleStatus403(const Packet403 *pkt403)
    {
      ESP_LOGD(TAG, "handleStatus403");
      if (!pkt403)
      {
        ESP_LOGE(TAG, "Invalid 403 packet");
        return;
      }
      if (packet403_)
      {
        this->packet403_->diff(pkt403);
        // settings applied from extractor to HA
        *this->packet403_ = *pkt403;
      }
      else
      {
        // skip first, let HA restore sates it knows
        this->packet403_ = std::make_unique<struct Packet403>(*pkt403);
      }
      if (this->status40x_pending_ == 0)
      {
        ESP_LOGD(TAG, "403 Status received and handling");
      }
      else
      {
        ESP_LOGW(TAG, "403 skipped %d", this->status40x_pending_);
      }
      handleSensors(this->packet403_.get());
    }

    void PurelinePro::handleStatus404(const Packet404 *pkt404)
    {
      ESP_LOGD(TAG, "handleStatus404");
      if (!pkt404)
      {
        ESP_LOGE(TAG, "Invalid 404 packet");
        return;
      }
      if (packet404_)
      {
        this->packet404_->diff(pkt404);
        // settings applied from extractor to HA
        *this->packet404_ = *pkt404;
      }
      else
      {
        // skip first, let HA restore sates it knows
        this->packet404_ = std::make_unique<struct Packet404>(*pkt404);
      }
      if (this->status40x_pending_ == 0)
      {
        ESP_LOGD(TAG, "404 Status received and handling");
      }
      else
      {
        ESP_LOGW(TAG, "404 skipped %d", this->status40x_pending_);
      }
      handleSensors(this->packet404_.get());
    }

    void PurelinePro::loop()
    {
      if (this->pending_request_ && (millis() > (last_request_ + 15 * 1000)))
      {
        // timeout....
        ESP_LOGW(TAG, "Timeout: %d", this->pending_request_);
        this->parent()->set_state(espbt::ClientState::CONNECTING);
        this->parent()->set_enabled(false);
        this->parent()->set_enabled(true);
      }

      if (millis() > (this->timer_ + 1 * 1000))
      {
        // some timer
        ESP_LOGV(TAG, "connected : %s %d", (this->node_state == espbt::ClientState::ESTABLISHED) ? "yes" : "no", this->pending_request_);
        timer_ = millis();
      }
    }

    void PurelinePro::update()
    {
      ESP_LOGV(TAG, "update : %s %d", (this->node_state == espbt::ClientState::ESTABLISHED) ? "yes" : "no", this->pending_request_);

      if (this->node_state != espbt::ClientState::ESTABLISHED)
      {
#ifdef USE_SWITCH
        if (!this->enabled_switch_ || this->enabled_switch_->state)
          ESP_LOGW(TAG, "!espbt::ClientState::ESTABLISHED");
#endif
        return;
      }

      if (this->pending_request_ != 0)
      {
        // we are still handling other requests
        return;
      }

      if (this->pending_request_ == 0)
      {
        ESP_LOGD(TAG, "Nothing is happening just ask status updates");
        this->request_status_update();

        if (this->status40x_count_++ > this->status40x_delay_)
        {
          this->request_status40x_update();
          this->status40x_count_ = 0;
        }
      }
    }

    void PurelinePro::setup()
    {
#ifdef USE_FAN
      if (this->extractor_fan_)
      {
        this->extractor_fan_->speed = 50; // initial default
        this->extractor_fan_->add_on_state_callback([this]()
                                                    {
                                                      ESP_LOGD(TAG, "extractor_fan_->add_on_state_callback");
                                                      if (this->auto_off_)
                                                        this->auto_off_ = false;

                                                      if (packet_)
                                                      {
                                                        bool request_status = false;
                                                        bool on = false;
                                                        if (packet_->getFanState() != this->extractor_fan_->state)
                                                        {
                                                          ESP_LOGI(TAG, "setting fan %d -> %d", packet_->getFanState(), this->extractor_fan_->state);
                                                          // when shitching 'on' we need to restore the speed....
                                                          on = this->extractor_fan_->state;

                                                          std::vector<uint8_t> payload = {1, this->extractor_fan_->state};
                                                          send_cmd(cmd_fan_state, payload, "fanstate");
                                                          request_status = true;
                                                        }
                                                        if (this->extractor_fan_->state)
                                                        {
                                                          if (on || (packet_->getFanSpeed() != this->extractor_fan_->speed))
                                                          {
                                                            ESP_LOGI(TAG, "setting fanspeed %d -> %d", packet_->getFanSpeed(), this->extractor_fan_->speed);

                                                            std::vector<uint8_t> payload = {1, (uint8_t)this->extractor_fan_->speed};
                                                            send_cmd(cmd_fan_speed, payload, "fanspeed");
                                                            request_status = true;
                                                          }
                                                        }
                                                        if (request_status)
                                                        {
                                                          ESP_LOGD(TAG, "Cmd's send, requesting status");
                                                          this->request_status_update();
                                                        }
                                                      } });
      }
#endif
#ifdef USE_LIGHT
      if (this->extractor_light_)
      {
        this->extractor_light_->add_on_state_callback([this]()
                                                      { 
                                                        ESP_LOGD(TAG, "this->extractor_light_->add_on_state_callback");
                                                        if(packet_)
                                                        {
                                                          bool request_status = false;
                                                          bool on = false;
                                                          if (packet_->getLightState() != this->extractor_light_->state_)
                                                          {
                                                            ESP_LOGD(TAG, "setting light %d -> %d", packet_->getLightState(), this->extractor_light_->state_);
                                                            // when shitching 'on' we need to restore the colors....
                                                            on = this->extractor_light_->state_;
                                                
                                                            // 36;0 is off
                                                            // both 15 (ambi) and 16 (white) are lights
                                                            std::vector<uint8_t> payload = {0};
                                                            send_cmd(this->extractor_light_->state_ ? cmd_light_on_white : cmd_light_off, payload, "lightstate");
                                                            request_status = true;
                                                          }
                                                          // Brightness
                                                          if (on || (this->extractor_light_->state_ && (packet_->getBrightness() != this->extractor_light_->raw_brightness_)))
                                                          {
                                                            ESP_LOGD(TAG, "setting light brigthness %d -> %d", packet_->getBrightness(), this->extractor_light_->raw_brightness_);
                                                
                                                            std::vector<uint8_t> payload = {1, this->extractor_light_->raw_brightness_};
                                                            send_cmd(cmd_light_brightness, payload, "brightness");
                                                            request_status = true;
                                                          }
                                                          // ColorTemp
                                                          if (on || (this->extractor_light_->state_ && (packet_->getColorTemp() != this->extractor_light_->raw_temp_)))
                                                          {
                                                            ESP_LOGD(TAG, "setting light temp %d -> %d", packet_->getColorTemp(), this->extractor_light_->raw_temp_);
                                                
                                                            std::vector<uint8_t> payload = {1, this->extractor_light_->raw_temp_};
                                                            send_cmd(cmd_light_colortemp, payload, "colortemp");
                                                            request_status = true;
                                                          }
                                                          if (request_status)
                                                          {
                                                            ESP_LOGD(TAG, "Cmd's send, requesting status");
                                                            this->request_status_update();
                                                          }
                                                        } });
      }
#endif
#ifdef USE_BUTTON
      if (this->power_button_)
      {
        this->power_button_->add_on_press_callback([this]()
                                                   {
                                                      if (this->node_state == espbt::ClientState::ESTABLISHED)
                                                      {
#ifdef USE_CMDS
                                                        // simulate power press
                                                        std::vector<uint8_t> payload = {0};
                                                        send_cmd(cmd_power, payload, "power");
#endif
                                                      } });
      }
      if (this->delayed_off_button_)
      {
        this->delayed_off_button_->add_on_press_callback([this]()
                                                        {
#ifdef USE_LIGHT
                                                          if (this->extractor_light_->state_)
                                                          {
                                                            this->extractor_light_->raw_brightness_ = 70;
                                                            this->extractor_light_->raw_temp_ = 255;
                                                            this->extractor_light_->publish(this->extractor_light_->state_, this->extractor_light_->raw_brightness_, this->extractor_light_->raw_temp_);
                                                          }
#endif
#ifdef USE_FAN
                                                          if (this->extractor_fan_->state && (this->extractor_fan_->speed > 25))
                                                          {
                                                            ESP_LOGI(TAG, "reducing fanspeed %d -> %d", this->extractor_fan_->speed, 25);
                                                            this->extractor_fan_->speed = 25;
                                                            this->extractor_fan_->publish_state();
                                                          }
                                                          if (this->extractor_fan_->state)
                                                          {
                                                            ESP_LOGI(TAG, "auto_off_timer_ started");

                                                            this->auto_off_timer_ = millis() + 5 * 60 * 1000; // run 5 more minutes
                                                            this->auto_off_ = true;
                                                          }
#endif
                                                        });
      }
      if (this->set_default_light_button_)
      {
        this->set_default_light_button_->add_on_press_callback([this]()
                                                          {
          if (this->node_state == espbt::ClientState::ESTABLISHED)
          {
#ifdef USE_CMDS
            // store default light
            std::vector<uint8_t> payload = {1,1};
            send_cmd(cmd_light_default, payload, "light_default");
#endif
          } });
      }
      if (this->set_default_speed_button_)
      {
        this->set_default_speed_button_->add_on_press_callback([this]()
                                                          {
          if (this->node_state == espbt::ClientState::ESTABLISHED)
          {
#ifdef USE_CMDS
            // store default fan speed
            std::vector<uint8_t> payload = {0};
            send_cmd(cmd_fan_default, payload, "fan_default");
#endif
          } });
      }
      if (this->reset_grease_button_)
      {
        this->reset_grease_button_->add_on_press_callback([this]()
                                                         {
          if (this->node_state == espbt::ClientState::ESTABLISHED)
          {
#ifdef USE_CMDS
            // simulate power press
            std::vector<uint8_t> payload = {0};
            send_cmd(cmd_reset_grease, payload, "resetgrease");
            pending_request_--;// this does not send an acknoladge
#endif
          } });
      }
#endif
#ifdef USE_SWITCH
      if (this->recirculate_switch_)
      {
        this->recirculate_switch_->add_on_state_callback([this](bool state)
                                                         {
          if (this->node_state == espbt::ClientState::ESTABLISHED)
          {
            if (this->packet402_)
            {
              if (this->packet402_->getRecirculate() != state)
              {
#ifdef USE_CMDS
                std::vector<uint8_t> payload = {1, state};
                send_cmd(cmd_fan_recirculate, payload, "recirculate");
#endif
              }
            }
          } });
      }
      if (this->enabled_switch_)
      {
        this->enabled_switch_->publish_state(true);
        this->enabled_switch_->add_on_state_callback([this](bool state)
                                                     { this->parent()->set_enabled(state); });
      }
#endif
    }

    void PurelinePro::gattc_event_handler(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if,
                                          esp_ble_gattc_cb_param_t *param)
    {
      switch (event)
      {
      case ESP_GATTC_DISCONNECT_EVT:
      {
        ESP_LOGI(TAG, "Disconnected");

        this->packet_.reset();
        this->packet402_.reset();
        this->packet403_.reset();
        this->packet404_.reset();

        this->rx_char_handle_ = 0;
        this->tx_char_handle_ = 0;

        this->status_pending_ = 0;
        this->status40x_pending_ = 0;
        this->status40x_cmd = cmd_hood_status402;

        this->status40x_count_ = 0;
        this->status40x_delay_ = 0;

        this->pending_request_ = 0;
        break;
      }

      case ESP_GATTC_OPEN_EVT:
      {
        if (param->open.status == ESP_GATT_OK)
        {
          ESP_LOGI(TAG, "Connected successfully!");
        }
        break;
      }

      case ESP_GATTC_SEARCH_CMPL_EVT:
      {
        ESP_LOGI(TAG, "ESP_GATTC_SEARCH_CMPL_EVT");
        auto tx_char = this->parent_->get_characteristic(uartServiceUUID, txCharUUID);
        if (tx_char == nullptr)
        {
          ESP_LOGE(TAG, "Not Novy PureLine Pro, sorry.");
          break;
        }
        this->tx_char_handle_ = tx_char->handle;

        auto rx_char = this->parent_->get_characteristic(uartServiceUUID, rxCharUUID);
        if (rx_char == nullptr)
        {
          ESP_LOGE(TAG, "Not Novy PureLine Pro, sorry.");
          break;
        }
        this->rx_char_handle_ = rx_char->handle;

        // register notify on tx char
        auto status = esp_ble_gattc_register_for_notify(this->parent_->get_gattc_if(), this->parent_->get_remote_bda(), tx_char->handle);
        if (status)
        {
          ESP_LOGW(TAG, "esp_ble_gattc_register_for_notify failed, status=%d", status);
        }

        ESP_LOGI(TAG, "Connected");

        break;
      }

      case ESP_GATTC_REG_FOR_NOTIFY_EVT:
      {
        this->node_state = espbt::ClientState::ESTABLISHED;
        ESP_LOGI(TAG, "Requesting initial status");
        request_status_update(); // initial
        request_status40x_update();
        break;
      }

      case ESP_GATTC_NOTIFY_EVT:
      {
        if (param->notify.handle != this->tx_char_handle_)
        {
          ESP_LOGW(TAG, "Received Unknown Notification: %d", param->notify.handle);
          break;
        }
        this->recieved_answer(param->notify.value, param->notify.value_len);
        break;
      }
      default:
        break;
      }
    }

    void PurelinePro::request_status_update()
    {
      std::vector<uint8_t> payload = {0};
      send_cmd(cmd_hood_status, payload, "state", false);
    }

    void PurelinePro::request_status40x_update()
    {
      std::vector<uint8_t> payload = {0};
      send_cmd(this->status40x_cmd, payload, "state", false);
    }

    void PurelinePro::send_cmd(int command_id, const std::vector<uint8_t> &args, const std::string &msg, bool log)
    {
      std::string payload = "[" + std::to_string(command_id);
      for (int arg : args)
      {
        payload += ";" + std::to_string(arg);
      }
      payload += "]";

      switch (command_id)
      {
      case cmd_hood_status:
        this->status_pending_++;
        break;
      case cmd_hood_status402:
      case cmd_hood_status403:
      case cmd_hood_status404:
        this->status40x_pending_++;
        break;
      }
      send_cmd(payload, msg, log);
    }

    void PurelinePro::send_cmd(std::string cmd, const std::string &msg, bool log)
    {
      if (this->node_state != espbt::ClientState::ESTABLISHED)
        return;

      uint8_t *raw_data = reinterpret_cast<uint8_t *>(cmd.data());
      size_t length = cmd.size();

      // Convert to std::vector<uint8_t>
      auto status = esp_ble_gattc_write_char(this->parent_->get_gattc_if(), this->parent_->get_conn_id(),
                                             this->rx_char_handle_, length, raw_data,
                                             ESP_GATT_WRITE_TYPE_RSP, ESP_GATT_AUTH_REQ_NONE);
      if (status)
      {
        ESP_LOGW(UARTTAG, "esp_ble_gattc_write_char, error = %d (%s)", status, cmd.data());
      }
      else
      {
        if (log)
          ESP_LOGI(UARTTAG, "Sent: %s (%s)", cmd.data(), msg.data());
        else
          ESP_LOGD(UARTTAG, "Sent: %s (%s)", cmd.data(), msg.data());
        this->last_request_ = millis();
        this->pending_request_++;
      }
    }

    void PurelinePro::recieved_answer(uint8_t *pData, uint16_t length)
    {
      this->pending_request_--;
      if (length && (pData[0] == '[') && (pData[length - 1] == ']'))
      {
        std::string_view sv(reinterpret_cast<const char *>(pData), length);

        ESP_LOGI(UARTTAG, "Received Reply: %.*s", static_cast<int>(sv.length()), sv.data());
        handleAck(sv);
      }
      else if (this->status_pending_ && (length == sizeof(Packet)))
      {
        ESP_LOGD(UARTTAG, "Received Status");
        this->status_pending_--;
        handleStatus400(reinterpret_cast<Packet *>(pData)); // reinterpret_cast
      }
      else if (this->status40x_pending_ && (this->status40x_cmd == cmd_hood_status402) && (length == sizeof(Packet402)))
      {
        ESP_LOGD(UARTTAG, "Received Status402");
        this->status40x_pending_--;
        handleStatus402(reinterpret_cast<Packet402 *>(pData)); // reinterpret_cast
        this->status40x_cmd++;                                 // other 40x status next time
      }
      else if (this->status40x_pending_ && (this->status40x_cmd == cmd_hood_status403) && (length == sizeof(Packet403)))
      {
        ESP_LOGD(UARTTAG, "Received Status403");
        this->status40x_pending_--;
        handleStatus403(reinterpret_cast<Packet403 *>(pData)); // reinterpret_cast
        this->status40x_cmd++;                                 // other 40x status next time
      }
      else if (this->status40x_pending_ && (this->status40x_cmd == cmd_hood_status404) && (length == sizeof(Packet404)))
      {
        ESP_LOGD(UARTTAG, "Received Status404");
        this->status40x_pending_--;
        handleStatus404(reinterpret_cast<Packet404 *>(pData)); // reinterpret_cast
        this->status40x_cmd = cmd_hood_status402;              // other 40x status next time
        this->status40x_delay_ = 5;                            // looped, from now on less frequent updates for these timers
      }
      else
      {
        // ESP_LOGI(UARTTAG, "sizeof(Packet402) %d", sizeof(Packet402));
        // ESP_LOGI(UARTTAG, "sizeof(Packet403) %d", sizeof(Packet403));
        // ESP_LOGI(UARTTAG, "sizeof(Packet404) %d", sizeof(Packet404));
        std::stringstream hexStream;
        hexStream << std::hex << std::setfill('0');
        for (uint16_t i = 0; i < length; ++i)
        {
          hexStream << std::setw(2) << static_cast<int>(pData[i]);
          if ((i + 1) % 20 == 0 && (i + 1) < length)
          {
            hexStream << "\n"; // Add newline for better readability in logs
          }
          else if ((i + 1) % 4 == 0 && (i + 1) < length)
          {
            hexStream << " "; // Add space for better readability
          }
        }
        ESP_LOGI(UARTTAG, "%s", hexStream.str().c_str());
      }
    }

    void PurelinePro::dump_config()
    {
      ESP_LOGCONFIG(TAG, "PurelinePro:");
#ifdef USE_FAN
      if (this->extractor_fan_)
        this->extractor_fan_->dump_config();
#endif
#ifdef USE_LIGHT
      if (this->extractor_light_)
        this->extractor_light_->dump_config();
#endif
#ifdef USE_BUTTON
      LOG_BUTTON("", "power", this->power_button_);
      LOG_BUTTON("", "delayed_off", this->delayed_off_button_);
      LOG_BUTTON("", "default_light", this->set_default_light_button_);
      LOG_BUTTON("", "default_speed", this->set_default_speed_button_);
      LOG_BUTTON("", "reset_grease", this->reset_grease_button_);
#endif
#ifdef USE_SWITCH
      LOG_SWITCH("", "recirculate", this->recirculate_switch_);
      LOG_SWITCH("", "enabled", this->enabled_switch_);
#endif
#ifdef USE_SENSOR
      LOG_SENSOR(TAG, "off_timer", this->off_timer_sensor_);
      LOG_SENSOR(TAG, "boost_timer", this->boost_timer_sensor_);
      LOG_SENSOR(TAG, "grease_timer", this->grease_timer_sensor_);
      LOG_SENSOR(TAG, "operating_hours_led", this->operating_hours_led_sensor_);
      LOG_SENSOR(TAG, "operating_hours_fan", this->operating_hours_fan_sensor_);
#endif
#ifdef USE_BINARY_SENSOR
      LOG_BINARY_SENSOR(TAG, "boost", this->boost_binary_sensor_);
      LOG_BINARY_SENSOR(TAG, "stopping", this->stopping_binary_sensor_);
#endif
    }
    /*
        void PurelinePro::control(const CoverCall &call) {
          if (this->node_state != espbt::ClientState::ESTABLISHED) {
            ESP_LOGW(TAG, "[%s] Cannot send cover control, not connected", this->get_name().c_str());
            return;
          }
          if (call.get_stop()) {
            auto *packet = this->encoder_->get_stop_request();
            auto status =
                esp_ble_gattc_write_char(this->parent_->get_gattc_if(), this->parent_->get_conn_id(), this->char_handle_,
                                         packet->length, packet->data, ESP_GATT_WRITE_TYPE_NO_RSP, ESP_GATT_AUTH_REQ_NONE);
            if (status) {
              ESP_LOGW(TAG, "[%s] Error writing stop command to device, error = %d", this->get_name().c_str(), status);
            }
          }
          if (call.get_position().has_value()) {
            auto pos = *call.get_position();

            if (this->invert_position_)
              pos = 1 - pos;
            auto *packet = this->encoder_->get_set_position_request(100 - (uint8_t) (pos * 100));
            auto status =
                esp_ble_gattc_write_char(this->parent_->get_gattc_if(), this->parent_->get_conn_id(), this->char_handle_,
                                         packet->length, packet->data, ESP_GATT_WRITE_TYPE_NO_RSP, ESP_GATT_AUTH_REQ_NONE);
            if (status) {
              ESP_LOGW(TAG, "[%s] Error writing set_position command to device, error = %d", this->get_name().c_str(), status);
            }
          }
        }
        */

  } // namespace am43
} // namespace esphome

#endif
