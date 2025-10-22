#include "esphome.h"
#ifdef USE_TIME
#include "esphome/core/time.h"
#endif
#include "ExtractorHood.h"

#define _TRACE_UNKNOWN_BITS_ 0

namespace esphome
{
  namespace purelinepro
  {
    const char *STATUSSTAG = "Status";

    const std::string lightMode[] = {"normal", "preset white", "preset ambi"};

    uint16_t swapEndian(uint16_t value) 
    {
      return (value >> 8) | (value << 8);
    }
    uint32_t swapEndian(uint32_t value) 
    {
      return ((value >> 24) & 0x000000FF) |
             ((value >> 8) & 0x0000FF00) |
             ((value << 8) & 0x00FF0000) |
             ((value << 24) & 0xFF000000);
    }

    bool Packet::getBoost() const
    {
      return flags1.flag1 && fanspeed > 75; // boost, will lower fan after timer....
    }
    bool Packet::getStopping() const
    {
      return flags1.flag1 && !getBoost(); // timer goes but not boost, so turning off
    }
    uint16_t Packet::getTimer() const
    {
      return flags1.flag1 ? swapEndian(countDown) : 0;
    }
    bool Packet::getCleanGreaseFilter() const
    {
      return flags2.flag0;
    }
    bool Packet::getLightState() const
    {
      return brightness > 0;//lightmode > 0;
    }
    uint8_t Packet::getBrightness() const
    {
      return brightness;
    }
    uint8_t Packet::getColorTemp() const
    {
      return colortemp;
    }
    uint8_t Packet::getLightMode() const
    {
      return lightmode;
    }

    bool Packet::getFanState() const
    {
      return fanspeed > 0;
    }
    uint8_t Packet::getFanSpeed() const
    {
      return fanspeed;
    }

    void Packet::print() const
    {
      ESP_LOGI(STATUSSTAG, "flags1: 0x%02X; flags2: 0x%02X", this->flags1.raw, this->flags2.raw);
      ESP_LOGI(STATUSSTAG, "speed: %u lightmode: %s, b:%u t:%u", this->fanspeed, lightMode[this->lightmode].c_str(), this->brightness, this->colortemp);
      ESP_LOGI(STATUSSTAG, "countDown: %u", this->getTimer());
      ESP_LOGI(STATUSSTAG, "unknown1 0x%04X (%u)", this->unknown1, this->unknown1);
      ESP_LOGI(STATUSSTAG, "unknown2 0x%02X (%u)", this->unknown2, this->unknown2);
      ESP_LOGI(STATUSSTAG, "unknown3 0x%04X (%u)", this->unknown3, this->unknown3);
      ESP_LOGI(STATUSSTAG, "unknown4 0x%04X (%u)", this->unknown4, this->unknown4);
      ESP_LOGI(STATUSSTAG, "unknown5 0x%04X (%u)", this->unknown5, this->unknown5);
    }

    void Packet::diff(const Packet *r) const
    {
      if (this->flags1.raw != r->flags1.raw)
        ESP_LOGI(STATUSSTAG, "flags1: 0x%02X", r->flags1.raw);
      if (this->flags2.raw != r->flags2.raw)
        ESP_LOGI(STATUSSTAG, "flags2: 0x%02X", r->flags2.raw);
      if ((this->fanspeed != r->fanspeed) || (this->lightmode != r->lightmode) || (this->brightness != r->brightness) || (this->colortemp != r->colortemp))
        ESP_LOGI(STATUSSTAG, "speed: %u lightmode: %s, b:%u t:%u", r->fanspeed, lightMode[r->lightmode].c_str(), r->brightness, r->colortemp);
      if (this->getTimer() != r->getTimer())
        if (r->getTimer() % 30 == 0) // do not print too many....
          ESP_LOGI(STATUSSTAG, "countDown: %u", r->getTimer());
      if (this->unknown1 != r->unknown1)
        ESP_LOGI(STATUSSTAG, "unknown1 0x%04X (%u)", r->unknown1, r->unknown1);
      if (this->unknown2 != r->unknown2)
        ESP_LOGI(STATUSSTAG, "unknown2 0x%02X (%u)", r->unknown2, r->unknown2);
      if (this->unknown3 != r->unknown3)
        ESP_LOGI(STATUSSTAG, "unknown3 0x%04X (%u)", r->unknown3, r->unknown3);
      if (this->unknown4 != r->unknown4)
        ESP_LOGI(STATUSSTAG, "unknown4 0x%04X (%u)", r->unknown4, r->unknown4);
      if (this->unknown5 != r->unknown5)
        ESP_LOGI(STATUSSTAG, "unknown5 0x%04X (%u)", r->unknown5, r->unknown5);
#if _TRACE_UNKNOWN_BITS_
      if (r->unknown1 != 0x00)
        ESP_LOGI(STATUSSTAG, "unknown1 0x%02X (%u)", r->unknown1, r->unknown1);
      if (r->unknown2 != 0xFF)
        ESP_LOGI(STATUSSTAG, "unknown2 0x%02X (%u)", r->unknown2, r->unknown2);
      if (r->unknown3 != 0x0100) // 0xFF00
        ESP_LOGI(STATUSSTAG, "unknown3 0x%04X (%u)", r->unknown3, r->unknown3);
      if (r->unknown4 != 0x00FF)
        ESP_LOGI(STATUSSTAG, "unknown4 0x%04X (%u)", r->unknown4, r->unknown4);
      if (r->unknown5 != 0x0000)
        ESP_LOGI(STATUSSTAG, "unknown5 0x%04X (%u)", r->unknown5, r->unknown5);
#endif
    }

    uint32_t Packet402::getGreaseTimer() const
    {
      return swapEndian(this->greasetime) / 60;
    }

    bool Packet402::getRecirculate() const
    {
      return flags.flag0;
    }

   void Packet402::print() const
   {
      ESP_LOGI(STATUSSTAG, "402");
      ESP_LOGI(STATUSSTAG, "flags: 0x%02X", this->flags.raw);
      ESP_LOGI(STATUSSTAG, "GreaseTimer: %u", this->getGreaseTimer());
      ESP_LOGI(STATUSSTAG, "Recirculate: %u", this->getRecirculate());
      ESP_LOGI(STATUSSTAG, "major 0x%02X (%u)", this->major, this->major);
      ESP_LOGI(STATUSSTAG, "minor 0x%02X (%u)", this->minor, this->minor);
      ESP_LOGI(STATUSSTAG, "patch 0x%02X (%u)", this->patch, this->patch);
      ESP_LOGI(STATUSSTAG, "unknown1 0x%04X (%u)", this->unknown1, this->unknown1);
      ESP_LOGI(STATUSSTAG, "unknown2 0x%02X (%u)", this->unknown2, this->unknown2);
      ESP_LOGI(STATUSSTAG, "unknown3 0x%02X (%u)", this->unknown3, this->unknown3);
      ESP_LOGI(STATUSSTAG, "unknown4 0x%04X (%u)", this->unknown4, this->unknown4);
      ESP_LOGI(STATUSSTAG, "unknown5 0x%08X (%u)", this->unknown5, this->unknown5);
  }

    void Packet402::diff(const Packet402 *r) const
    {
      if (this->flags.raw != r->flags.raw)
        ESP_LOGI(STATUSSTAG, "flags: 0x%02X", r->flags.raw);
      if (this->getGreaseTimer() != r->getGreaseTimer())
        ESP_LOGI(STATUSSTAG, "GreaseTimer: %u", r->getGreaseTimer());
      if (this->getRecirculate() != r->getRecirculate())
        ESP_LOGI(STATUSSTAG, "Recirculate: %u", r->getRecirculate());
      if (this->major != r->major)
        ESP_LOGI(STATUSSTAG, "major 0x%02X (%u)", r->major, r->major);
      if (this->minor != r->minor)
        ESP_LOGI(STATUSSTAG, "minor 0x%02X (%u)", r->minor, r->minor);
      if (this->patch != r->patch)
        ESP_LOGI(STATUSSTAG, "patch 0x%02X (%u)", r->patch, r->patch);

      if (this->unknown1 != r->unknown1)
        ESP_LOGI(STATUSSTAG, "402 unknown1 0x%04X (%u)", r->unknown1, r->unknown1);
      if (this->unknown2 != r->unknown2)
        ESP_LOGI(STATUSSTAG, "402 unknown2 0x%02X (%u)", r->unknown2, r->unknown2);
      if (this->unknown3 != r->unknown3)
        ESP_LOGI(STATUSSTAG, "402 unknown3 0x%02X (%u)", r->unknown3, r->unknown3);
      if (this->unknown4 != r->unknown4)
        ESP_LOGI(STATUSSTAG, "402 unknown4 0x%04X (%u)", r->unknown4, r->unknown4);
      if (this->unknown5 != r->unknown5)
        ESP_LOGI(STATUSSTAG, "402 unknown5 0x%08X (%u)", r->unknown5, r->unknown5);
#if _TRACE_UNKNOWN_BITS_
      if (r->unknown1 != 0x6419) // 0x640C
        ESP_LOGI(STATUSSTAG, "402 unknown1 0x%04X (%u)", r->unknown1, r->unknown1);
      if (r->unknown2 != 0x0ff)
        ESP_LOGI(STATUSSTAG, "402 unknown2 0x%02X (%u)", r->unknown2, r->unknown2);
      if (r->unknown3 != 0x00) // 0x04
        ESP_LOGI(STATUSSTAG, "402 unknown3 0x%02X (%u)", r->unknown3, r->unknown3);
      if (r->unknown4 != 0x0000) // 0x1E1B0000
        ESP_LOGI(STATUSSTAG, "402 unknown4 0x%04X (%u)", r->unknown4, r->unknown4);
      if (r->unknown5 != 0x0000) // 0x1E1B0000
        ESP_LOGI(STATUSSTAG, "402 unknown5 0x%04X (%u)", r->unknown5, r->unknown5);
#endif
    }

    uint8_t Packet403::getSwitchOffFanSpeed() const
    {
      return this->switchOffFanSpeed;
    }

    uint32_t Packet403::getAnotherTimer() const
    {
      return swapEndian(this->anotherTimer) / 60;
    }

    uint32_t Packet403::getRecirculateTimer() const
    {
      return swapEndian(this->recirculateTimer) / 60;
    }

    uint32_t Packet403::getFanTimer() const
    {
      return swapEndian(this->fanTimer) / 60;
    }

    uint8_t Packet403::getFanSpeed() const
    {
      return fanSpeed;
    }
    uint8_t Packet403::getFunctionalBrightness() const
    {
      return functionalBrightness;
    }
    uint8_t Packet403::getFunctionalColorTemp() const
    {
      return functionalColor;
    }
    uint8_t Packet403::getAmbiBrightness() const
    {
      return ambiBrightness;
    }
    uint8_t Packet403::getAmbiColorTemp() const
    {
      return ambiColor;
    }
    uint8_t Packet403::getBrightness(bool ambi) const
    {
      return ambi ? getAmbiBrightness() : getFunctionalBrightness();
    }
    uint8_t Packet403::getColorTemp(bool ambi) const
    {
      return ambi ? getAmbiColorTemp() : getFunctionalColorTemp();
    }

    void Packet403::print() const
    {
      ESP_LOGI(STATUSSTAG, "403");
      ESP_LOGI(STATUSSTAG, "getSwitchOffFanSpeed: %u", this->getSwitchOffFanSpeed());
      ESP_LOGI(STATUSSTAG, "AnotherTimer: %u", this->getAnotherTimer());
      ESP_LOGI(STATUSSTAG, "RecirculateTimer: %u", this->getRecirculateTimer());
      ESP_LOGI(STATUSSTAG, "FanTimer: %u", this->getFanTimer());
      ESP_LOGI(STATUSSTAG, "FanSpeed %u", this->getFanSpeed());
      ESP_LOGI(STATUSSTAG, "FunctionalBrightness %u", this->getFunctionalBrightness());
      ESP_LOGI(STATUSSTAG, "FunctionalColor %u", this->getFunctionalColorTemp());
      ESP_LOGI(STATUSSTAG, "AmbiBrightness %u", this->getAmbiBrightness());
      ESP_LOGI(STATUSSTAG, "AmbiColor %u", this->getAmbiColorTemp());
      ESP_LOGI(STATUSSTAG, "unknown1 0x%02X (%u)", this->unknown1, this->unknown1);
      ESP_LOGI(STATUSSTAG, "unknown2 0x%02X (%u)", this->unknown2, this->unknown2);
    }

    void Packet403::diff(const Packet403 *r) const
    {
      if (this->getSwitchOffFanSpeed() != r->getSwitchOffFanSpeed())
        ESP_LOGI(STATUSSTAG, "getSwitchOffFanSpeed: %u", r->getSwitchOffFanSpeed());
      if (this->getAnotherTimer() != r->getAnotherTimer())
        ESP_LOGI(STATUSSTAG, "AnotherTimer: %u", r->getAnotherTimer());
      if (this->getRecirculateTimer() != r->getRecirculateTimer())
        ESP_LOGI(STATUSSTAG, "RecirculateTimer: %u", r->getRecirculateTimer());
      if (this->getFanTimer() != r->getFanTimer())
        ESP_LOGI(STATUSSTAG, "FanTimer: %u", r->getFanTimer());
      if (this->getFanSpeed() != r->getFanSpeed())
        ESP_LOGI(STATUSSTAG, "FanSpeed %u", r->getFanSpeed());
      if (this->getFunctionalBrightness() != r->getFunctionalBrightness())
        ESP_LOGI(STATUSSTAG, "FunctionalBrightness %u", r->getFunctionalBrightness());
      if (this->getFunctionalColorTemp() != r->getFunctionalColorTemp())
        ESP_LOGI(STATUSSTAG, "FunctionalColor %u", r->getFunctionalColorTemp());
      if (this->getAmbiBrightness() != r->getAmbiBrightness())
        ESP_LOGI(STATUSSTAG, "AmbiBrightness %u", r->getAmbiBrightness());
      if (this->getAmbiColorTemp() != r->getAmbiColorTemp())
        ESP_LOGI(STATUSSTAG, "AmbiColor %u", r->getAmbiColorTemp());

      if (this->unknown1 != r->unknown1)
        ESP_LOGI(STATUSSTAG, "403 unknown1 0x%02X (%u)", r->unknown1, r->unknown1);
      if (this->unknown2 != r->unknown2)
        ESP_LOGI(STATUSSTAG, "403 unknown2 0x%02X (%u)", r->unknown2, r->unknown2);
#if _TRACE_UNKNOWN_BITS_
      if (this->getSwitchOffFanSpeed() != 0x19) // not sure about it....
        ESP_LOGI(STATUSSTAG, "403 getSwitchOffFanSpeed 0x%02X (%u)", r->getSwitchOffFanSpeed(), r->getSwitchOffFanSpeed());
      if (r->unknown1 != 0x00) // 0x02
        ESP_LOGI(STATUSSTAG, "403 unknown1 0x%02X (%u)", r->unknown1, r->unknown1);
      if (r->unknown2 != 0x19)
        ESP_LOGI(STATUSSTAG, "403 unknown2 0x%02X (%u)", r->unknown2, r->unknown2);
#endif
    }

    uint32_t Packet404::getLedTimer() const
    {
      return swapEndian(this->ledtimer) / 60;
    }

    void Packet404::print() const
    {
      ESP_LOGI(STATUSSTAG, "404");
      ESP_LOGI(STATUSSTAG, "LedTimer: %u", this->getLedTimer());
      ESP_LOGI(STATUSSTAG, "unknown1 0x%08X (%u)", this->unknown1, this->unknown1);
      ESP_LOGI(STATUSSTAG, "unknown2 0x%08X (%u)", this->unknown2, this->unknown2);
      ESP_LOGI(STATUSSTAG, "unknown3 0x%08X (%u)", this->unknown3, this->unknown3);
      ESP_LOGI(STATUSSTAG, "unknown4 0x%04X (%u)", this->unknown4, this->unknown4);
      ESP_LOGI(STATUSSTAG, "unknown5 0x%02X (%u)", this->unknown5, this->unknown5);
      ESP_LOGI(STATUSSTAG, "unknown6 0x%04X (%u)", this->unknown6, this->unknown6);
    }

    void Packet404::diff(const Packet404 *r) const
    {
      if (this->getLedTimer() != r->getLedTimer())
        ESP_LOGI(STATUSSTAG, "LedTimer: %u", r->getLedTimer());
      if (this->unknown1 != r->unknown1)
        ESP_LOGI(STATUSSTAG, "404 unknown1 0x%08X (%u)", r->unknown1, r->unknown1);
      if (this->unknown2 != r->unknown2)
        ESP_LOGI(STATUSSTAG, "404 unknown2 0x%08X (%u)", r->unknown2, r->unknown2);
      if (this->unknown3 != r->unknown3)
        ESP_LOGI(STATUSSTAG, "404 unknown3 0x%08X (%u)", r->unknown3, r->unknown3);
      if (this->unknown4 != r->unknown4)
        ESP_LOGI(STATUSSTAG, "404 unknown4 0x%04X (%u)", r->unknown4, r->unknown4);
      if (this->unknown5 != r->unknown5)
        ESP_LOGI(STATUSSTAG, "404 unknown5 0x%02X (%u)", r->unknown5, r->unknown5);
      if (this->unknown6 != r->unknown6)
        ESP_LOGI(STATUSSTAG, "404 unknown6 0x%04X (%u)", r->unknown6, r->unknown6);
#if _TRACE_UNKNOWN_BITS_
      if (r->unknown1 != 0x00000000)
        ESP_LOGI(STATUSSTAG, "404 unknown1 0x%08X (%u)", r->unknown1, r->unknown1);
      if (r->unknown2 != 0x1901FF00) // off delay? channel/ rssi
        ESP_LOGI(STATUSSTAG, "404 unknown2 0x%08X (%u)", r->unknown2, r->unknown2);
      if (r->unknown3 != 0x00000040) // 
        ESP_LOGI(STATUSSTAG, "404 unknown3 0x%08X (%u)", r->unknown3, r->unknown3);
      if (r->unknown4 != 0x0000) // 0x0075
        ESP_LOGI(STATUSSTAG, "404 unknown4 0x%04X (%u)", r->unknown4, r->unknown4);
      if (r->unknown5 != 0x00)
        ESP_LOGI(STATUSSTAG, "404 unknown5 0x%02X (%u)", r->unknown5, r->unknown5);
      if (r->unknown6 != 0x0000)
        ESP_LOGI(STATUSSTAG, "404 unknown6 0x%04X (%u)", r->unknown6, r->unknown6);
#endif
    }
  } // namespace purelinepro
} // namespace esphome
