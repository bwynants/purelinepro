#pragma once

#include "esphome/core/component.h"
#include "esphome/components/fan/fan.h"

namespace esphome
{
  namespace purelinepro
  {

    extern const char *TAG;

    class ExtractorFan : public fan::Fan, public Component
    {
    public:
      fan::FanTraits get_traits() override
      {
        fan::FanTraits traits{};

        traits.set_speed(true);
        traits.set_supported_speed_count(100);

        return traits;
      }
      void set_preset_modes(const std::set<std::string> &presets) { this->preset_modes_ = presets; }

    protected:
      void control(const fan::FanCall &call) override
      {
        if (call.get_state().has_value())
          this->state = *call.get_state();
        if (call.get_oscillating().has_value())
          this->oscillating = *call.get_oscillating();
        if (call.get_speed().has_value())
          this->speed = *call.get_speed();
        if (call.get_direction().has_value())
          this->direction = *call.get_direction();

        this->publish_state();
      }
      std::set<std::string> preset_modes_{};
    };

  } // namespace purelinepro
} // namespace esphome
