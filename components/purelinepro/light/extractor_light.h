#pragma once

#include "esphome/core/component.h"

namespace esphome
{
  namespace purelinepro
  {

    extern const char *TAG;

    class ExtractorLight : public light::LightOutput, public Component
    {
    public:
        const float min_mireds = 154.0f;//oldest (6500K)
        const float max_mireds = 370.0f;//warmest (2700K)
      light::LightTraits get_traits() override
      {

        light::LightTraits traits{};
        traits.set_supported_color_modes({light::ColorMode::COLD_WARM_WHITE});
        traits.set_min_mireds(min_mireds);
        traits.set_max_mireds(max_mireds);
        return traits;
      }

      uint8_t to_raw_brightness(float brightness)
      {
        return static_cast<uint8_t>(brightness * 255.0f + 0.5f);
      }
      uint8_t to_raw_temp(float mireds)
      {
        // Clamp mireds to valid range
        mireds = std::clamp(mireds, min_mireds, max_mireds);

        return static_cast<uint8_t>(roundf((mireds - min_mireds) * 255.0f / (max_mireds - min_mireds)));
      }
      float to_brightness(uint8_t raw_brightness)
      {
        return raw_brightness_ / 255.0f;
      }
      float to_mireds(uint8_t raw_temp)
      {
        return min_mireds + (float(raw_temp) / 255.0f) * (max_mireds - min_mireds);
      }

      void setup_state(light::LightState *state) override
      {
        light::LightOutput::setup_state(state);

        lstate_ = state;

        state_ = state->current_values.is_on();
        raw_brightness_ = to_raw_brightness(state->current_values.get_brightness());
        raw_temp_ = to_raw_temp(state->current_values.get_color_temperature());

        ESP_LOGD(TAG, "restored: brightness %u, temp %u", raw_brightness_, raw_temp_);
      };

      void write_state(light::LightState *state) override
      {
        state_ = state->current_values.is_on();

        if (!state->current_values.is_on())
        {
          this->state_callback_.call();
          return;
        }

        // Extract brightness and color temp
        raw_brightness_ = to_raw_brightness(state->current_values.get_brightness());
        raw_temp_ = to_raw_temp(state->current_values.get_color_temperature());

        ESP_LOGD(TAG, "HA -> Hood: brightness %u, temp %u", raw_brightness_, raw_temp_);

        // Send state to frontend
        this->state_callback_.call();
      }

      // Called when extractor sends updates
      void publish(bool state, uint8_t brightness, uint8_t color_temp)
      {
        state_ = state;
        raw_brightness_ = brightness;
        raw_temp_ = color_temp;

        ESP_LOGD(TAG, "Hood -> HA state :%d, brightness %.2f(%u), mireds %.2f(%u)", state, to_brightness(raw_brightness_), raw_brightness_, to_mireds(raw_temp_), raw_temp_);

        update_esp_light();
      }

      void add_on_state_callback(std::function<void()> &&callback)
      {
        this->state_callback_.add(std::move(callback));
      }

      bool state_ = false;
      uint8_t raw_brightness_ = to_raw_brightness(0);
      uint8_t raw_temp_ = to_raw_temp(370.0f);

    protected:
      void update_esp_light()
      {
        auto call = this->lstate_->make_call();
        call.set_state(state_);
        call.set_brightness(to_brightness(raw_brightness_));
        call.set_color_temperature(to_mireds(raw_temp_));
        call.perform();
      }

      light::LightState *lstate_{nullptr};

      CallbackManager<void()> state_callback_{};
    };

  } // namespace purelinepro
} // namespace esphome
