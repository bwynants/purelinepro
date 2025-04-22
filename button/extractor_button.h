#pragma once

#include "esphome/core/component.h"
#include "esphome/components/button/button.h"

namespace esphome
{
  namespace purelinepro
  {
    extern const char *TAG;

    class ExtractorButton : public button::Button, public Component
    {
    public:
      void dump_config() { 
        LOG_BUTTON(TAG, "Extractor Button", this);
      }

    protected:
      void press_action()
      {
      }
    };

  } // namespace shutdown
} // namespace esphome
