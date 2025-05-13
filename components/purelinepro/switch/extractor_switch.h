#pragma once

#include "esphome/core/component.h"
#include "esphome/components/switch/switch.h"

#ifdef USE_ESP32

namespace esphome
{
  namespace purelinepro
  {
    extern const char *TAG;

    class ExtractorSwitch : public switch_::Switch, public Component
    {
    public:
      void setup() override
      {
        ESP_LOGCONFIG(TAG, "Setting up Extractor Switch '%s'...", this->name_.c_str());

        bool initial_state = false;

        if (initial_state)
        {
          this->turn_on();
        }
        else
        {
          this->turn_off();
        }
      }

    protected:
      void write_state(bool state) override
      {
        this->publish_state(state);
      }
    };
#endif

  } // namespace purelinepro
} // namespace esphome