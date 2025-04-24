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
    const int cmd_hood_extrastatus = 402;

    void PurelinePro::handleAck(std::string_view ack)
    {
    }

    void PurelinePro::handleSensors(const Packet *pkt)
    {
      ESP_LOGD(TAG, "handleSensors");
      // we have our own timer and the hood has a timer, combine those in reporting to HA
      // work with the timer....
      uint32_t auto_off_timer = pkt->getTimer();

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
      if (this->timer_sensor_)
      {
        if (this->timer_sensor_->state != auto_off_timer)
          this->timer_sensor_->publish_state(auto_off_timer);
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

    void PurelinePro::handleSensors(const ExtraPacket *pkt)
    {
      ESP_LOGD(TAG, "handleSensors");
#ifdef USE_SENSOR
      if (this->greasetimer_sensor_)
      {
        if (this->greasetimer_sensor_->state != pkt->getGreaseTimer())
          this->greasetimer_sensor_->publish_state(pkt->getGreaseTimer());
      }
#endif
    }

    void PurelinePro::handleSwitch(const ExtraPacket *pkt)
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
          ESP_LOGD(TAG, "setting HA fan %d -> %d", packet_->getFanState(), this->extractor_fan_->state);
          publish = true;
          this->extractor_fan_->state = pkt->getFanState(); // give to HA, no changing to extractor
        }
        if (pkt->getFanState())
        {
          if (this->extractor_fan_->speed != pkt->getFanSpeed())
          {
            publish = true;
            ESP_LOGD(TAG, "setting HA fan speed %d -> %d", packet_->getFanSpeed(), this->extractor_fan_->speed);
            this->extractor_fan_->speed = pkt->getFanSpeed(); // give to HA, no changing to extractor
          }
        }
        if (publish)
        {
          if (this->auto_off_)
            ESP_LOGI(TAG, "auto_off_timer_ stopped");
          this->auto_off_ = false; // stop running
          ESP_LOGI(TAG, "setting fanspeed from pkt %d -> %d", this->extractor_fan_->speed, pkt->getFanSpeed());
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
          ESP_LOGI(TAG, "setting HA light %d -> %d", packet_->getLightState(), this->extractor_light_->state_);
          this->extractor_light_->state_ = pkt->getLightState();
          publish = true;
        }
        if (pkt->getLightState())
        {
          if (this->extractor_light_->raw_brightness_ != pkt->getBrightness())
          {
            ESP_LOGI(TAG, "setting HA light B %d -> %d", packet_->getBrightness(), this->extractor_light_->raw_brightness_);
            this->extractor_light_->raw_brightness_ = pkt->getBrightness();
            publish = true;
          }
          if (this->extractor_light_->raw_temp_ != pkt->getColorTemp())
          {
            ESP_LOGI(TAG, "setting HA light T %d -> %d", packet_->getColorTemp(), this->extractor_light_->raw_temp_);
            this->extractor_light_->raw_temp_ = pkt->getColorTemp();
            publish = true;
          }
        }
        if (publish)
          this->extractor_light_->publish(this->extractor_light_->state_, this->extractor_light_->raw_brightness_, this->extractor_light_->raw_temp_);
      }
#endif
    }

    void PurelinePro::handleStatus(const Packet *pkt)
    {
      ESP_LOGD(TAG, "handleStatus");
      if (!pkt)
      {
        ESP_LOGE(TAG, "Invalid packet");
        return;
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
      // update sensors always, independend of status_pending_
      handleSensors(this->packet_.get());
    }

    void PurelinePro::handleExtraStatus(const ExtraPacket *extraPkt)
    {
      ESP_LOGD(TAG, "handleExtraStatus");
      if (!extraPkt)
      {
        ESP_LOGE(TAG, "Invalid packet");
        return;
      }
      if (this->extraStatus_pending_ == 0)
      {
        ESP_LOGD(TAG, "Extra Status received and handling");
        handleSensors(extraPkt);
        handleSwitch(extraPkt);
      }
      else
      {
        ESP_LOGW(TAG, "Extra skipped %d", this->extraStatus_pending_);
      }
      if (extraPacket_)
      {
        this->extraPacket_->diff(extraPkt);
        // settings applied from extractor to HA
        *this->extraPacket_ = *extraPkt;
      }
      else
      {
        // skip first, let HA restore sates it knows
        this->extraPacket_ = std::make_unique<struct ExtraPacket>(*extraPkt);
      }
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
        this->request_statusupdate();

        if (this->extraStatus_count_++ > 10)
        {
          this->request_extrastatusupdate();
          this->extraStatus_count_ = 0;
        }
      }
    }

    void PurelinePro::setup()
    {
#ifdef USE_SENSOR
      //if (this->timer_sensor_)
      //  this->timer_sensor_->publish_state(0);
      //if (this->greasetimer_sensor_)
      //  this->greasetimer_sensor_->publish_state(20);// default is 20 hours
#endif
#ifdef USE_BINARY_SENSOR
      //if (this->boost_binary_sensor_)
      //  this->boost_binary_sensor_->publish_state(0);
      //if (this->stopping_binary_sensor_)
      //  this->stopping_binary_sensor_->publish_state(0);
#endif
#ifdef USE_FAN
      if (this->extractor_fan_)
      {
        this->extractor_fan_->speed = 50; // initial default
        this->extractor_fan_->add_on_state_callback([this]()
                                                    {
                                                      ESP_LOGD(TAG, "extractor_fan_->add_on_state_callback");
                                                      if (this->auto_off_)
                                                        this->auto_off_ = false;

                                                      if(packet_)
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
                                                          this->request_statusupdate();
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
                                                            this->request_statusupdate();
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
      if (this->timedoff_button_)
      {
        this->timedoff_button_->add_on_press_callback([this]()
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
      if (this->defaultlight_button_)
      {
        this->defaultlight_button_->add_on_press_callback([this]()
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
      if (this->defaultspeed_button_)
      {
        this->defaultspeed_button_->add_on_press_callback([this]()
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
      if (this->resetgrease_button_)
      {
        this->resetgrease_button_->add_on_press_callback([this]()
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
            if (this->extraPacket_)
            {
              if (this->extraPacket_->getRecirculate() != state)
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

        // make sure we send HA changes back to hood!
        // if an automation turned it on/off it must be in that state....
        this->packet_.reset();
        this->extraPacket_.reset();

        this->rx_char_handle_ = 0;
        this->tx_char_handle_ = 0;

        this->status_pending_ = 0;
        this->extraStatus_pending_ = 0;
        this->extraStatus_count_ = 0;

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
        request_statusupdate(); // initial
        request_extrastatusupdate();
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

    void PurelinePro::request_statusupdate()
    {
      std::vector<uint8_t> payload = {0};
      send_cmd(cmd_hood_status, payload, "state", false);
    }

    void PurelinePro::request_extrastatusupdate()
    {
      std::vector<uint8_t> payload = {0};
      send_cmd(cmd_hood_extrastatus, payload, "state", false);
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
      case cmd_hood_extrastatus:
        this->extraStatus_pending_++;
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

        ESP_LOGD(UARTTAG, "Received Reply: %.*s", static_cast<int>(sv.length()), sv.data());
        handleAck(sv);
      }
      else if (this->status_pending_ && (length == sizeof(Packet)))
      {
        ESP_LOGD(UARTTAG, "Received Status");
        this->status_pending_--;
        handleStatus(reinterpret_cast<Packet *>(pData)); // reinterpret_cast
      }
      else if (this->extraStatus_pending_ && (length == sizeof(ExtraPacket)))
      {
        ESP_LOGD(UARTTAG, "Received ExtraStatus");
        this->extraStatus_pending_--;
        handleExtraStatus(reinterpret_cast<ExtraPacket *>(pData)); // reinterpret_cast
      }
      else
      {
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
      LOG_BUTTON("", "timedoff", this->timedoff_button_);
      LOG_BUTTON("", "defaultlight", this->defaultlight_button_);
      LOG_BUTTON("", "defaultspeed", this->defaultspeed_button_);
      LOG_BUTTON("", "resetgrease", this->resetgrease_button_);
#endif
#ifdef USE_SWITCH
      LOG_SWITCH("", "recirculate", this->recirculate_switch_);
      LOG_SWITCH("", "enabled", this->enabled_switch_);
#endif
#ifdef USE_SENSOR
      LOG_SENSOR(TAG, " timer", this->timer_sensor_);
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
