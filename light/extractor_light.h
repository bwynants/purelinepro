#pragma once

#include "esphome/core/component.h"
#include "esphome/components/light/light_output.h"

namespace esphome
{
  namespace purelinepro
  {

    extern const char *TAG;

    class ExtractorLight : public light::LightOutput, public Component
    {
    public:
      light::LightTraits get_traits() override
      {
        light::LightTraits traits{};
        traits.set_supported_color_modes({light::ColorMode::COLD_WARM_WHITE});
        traits.set_min_mireds(153);
        traits.set_max_mireds(500);
        return traits;
      }

      void setup_state(light::LightState *state) override { lstate_ = state; };

      void write_state(light::LightState *state) override
      {
        if (!state->current_values.is_on())
        {
          state_ = false;

          this->state_callback_.call();
          return;
        }

        state_ = true;

        // Extract brightness and color temp
        float brightness = state->current_values.get_brightness();    // 0.0–1.0
        float mireds = state->current_values.get_color_temperature(); // 153–500

        raw_brightness_ = static_cast<uint8_t>(brightness * 255.0f);
        float pct = (500.0f - mireds) / (500.0f - 153.0f); // reverse mapping
        raw_temp_ = 255-static_cast<uint8_t>(pct * 255.0f);

        ESP_LOGI("remote_light", "HA -> Hood: brightness %u, temp %u", raw_brightness_, raw_temp_);

        // Send state to frontend
        this->state_callback_.call();
      }

      // Called when extractor sends updates
      void set_raw(bool state, uint8_t brightness, uint8_t color_temp)
      {
        state_ = state;
        raw_brightness_ = brightness;
        raw_temp_ = color_temp;

        brightness_ = raw_brightness_ / 255.0f;
        color_temp_ = 153.0f + (500.0f - 153.0f) * (/*1.0f - */raw_temp_ / 255.0f);
        ESP_LOGI("remote_light", "Hood -> state :%d, brightness %.2f, temp %.2f", state, brightness_, color_temp_);

        update_esp_light();
      }

      void add_on_state_callback(std::function<void()> &&callback)
      {
        this->state_callback_.add(std::move(callback));
      }

      bool state_ = false;
      uint8_t raw_brightness_ = 0;
      uint8_t raw_temp_ = 0;

    protected:
      float brightness_ = 1.0f;
      float color_temp_ = 370.0f;

      void update_esp_light()
      {
        auto call = this->lstate_->make_call();
        call.set_state(true);
        call.set_brightness(brightness_);
        call.set_color_temperature(color_temp_);
        call.perform();
      }

      light::LightState *lstate_{nullptr};

      CallbackManager<void()> state_callback_{};
    };

  } // namespace purelinepro
} // namespace esphome
