#include "esphome.h"
#ifdef USE_TIME
#include "esphome/core/time.h"
#endif
#include "ExtractorHood.h"

namespace esphome
{
  namespace purelinepro
  {
    const char *STATUSSTAG = "Status";

    const std::string lightMode[] = {"off", "white", "ambi"};

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
      return flag1 && fanspeed > 75; // boost, will lower fan after timer....
    }
    bool Packet::getStopping() const
    {
      return flag1 && !getBoost(); // timer goes but not boost, so turning off
    }
    uint16_t Packet::getTimer() const
    {
      return flag1 ? swapEndian(countDown) : 0;
    }

    bool Packet::getLightState() const
    {
      return lightmode > 0;
    }
    uint8_t Packet::getBrightness() const
    {
      return brightness;
    }
    uint8_t Packet::getColorTemp() const
    {
      return colortemp;
    }

    bool Packet::getFanState() const
    {
      return fanspeed > 0;
    }
    uint8_t Packet::getFanSpeed() const
    {
      return fanspeed;
    }

    void Packet::diff(const Packet *r) const
    {
      if ((this->flag0 != r->flag0) || (this->flag1 != r->flag1) || (this->flag2 != r->flag2) || (this->flag3 != r->flag3) || (this->flag4 != r->flag4) || (this->flag5 != r->flag5) || (this->flag6 != r->flag6) || (this->flag7 != r->flag7))
        ESP_LOGI(STATUSSTAG, "flags: %d,%d,%d,%d,%d,%d,%d,%d", r->flag0, r->flag1, r->flag2, r->flag3, r->flag4, r->flag5, r->flag6, r->flag7);
      if ((this->fanspeed != r->fanspeed) || (this->lightmode != r->lightmode) || (this->brightness != r->brightness) || (this->colortemp != r->colortemp))
        ESP_LOGI(STATUSSTAG, "speed: %d lightmode: %s, b:%d t:%d", r->fanspeed, lightMode[r->lightmode].c_str(), r->brightness, r->colortemp);
      if (this->getTimer() != r->getTimer())
        if (r->getTimer() % 30 == 0) // do not print too many....
          ESP_LOGI(STATUSSTAG, "countDown: %d", r->getTimer());
      if (this->unknown1 != r->unknown1)
        ESP_LOGI(STATUSSTAG, "unknown1 0x%04X (%d)", r->unknown1, r->unknown1);
      if (this->unknown2 != r->unknown2)
        ESP_LOGI(STATUSSTAG, "unknown2 0x%02X (%d)", r->unknown2, r->unknown2);
      if (this->unknown3 != r->unknown3)
        ESP_LOGI(STATUSSTAG, "unknown3 0x%04X (%d)", r->unknown3, r->unknown3);
      if (this->unknown4 != r->unknown4)
        ESP_LOGI(STATUSSTAG, "unknown4 0x%04X (%d)", r->unknown4, r->unknown4);
      if (this->unknown5 != r->unknown5)
        ESP_LOGI(STATUSSTAG, "unknown5 0x%04X (%d)", r->unknown5, r->unknown5);
#if 1
      if (r->unknown1 != 0x00)
        ESP_LOGI(STATUSSTAG, "unknown1 0x%04X (%d)", r->unknown1, r->unknown1);
      if (r->unknown2 != 0xFF)
        ESP_LOGI(STATUSSTAG, "unknown2 0x%02X (%d)", r->unknown2, r->unknown2);
      if (r->unknown3 != 0x0100)
        ESP_LOGI(STATUSSTAG, "unknown3 0x%04X (%d)", r->unknown3, r->unknown3);
      if (r->unknown4 != 0x00FF)
        ESP_LOGI(STATUSSTAG, "unknown4 0x%04X (%d)", r->unknown4, r->unknown4);
      if (r->unknown5 != 0x0000)
        ESP_LOGI(STATUSSTAG, "unknown5 0x%04X (%d)", r->unknown5, r->unknown5);
#endif
    }

    bool operator==(const Packet &l, const Packet &r)
    {
      return (l.fanspeed == r.fanspeed) && (l.lightmode == r.lightmode) && (l.brightness == r.brightness) && (l.colortemp == r.colortemp) && (l.countDown == r.countDown) &&
             (l.flag0 == r.flag0) && (l.flag1 == r.flag1) && (l.flag2 == r.flag2) && (l.flag3 == r.flag3) && (l.flag4 == r.flag4) && (l.flag5 == r.flag5) && (l.flag6 == r.flag6) && (l.flag7 == r.flag7) &&
             (l.unknown1 == r.unknown1) && (l.unknown2 == r.unknown2) && (l.unknown3 == r.unknown3) && (l.unknown4 == r.unknown4) && (l.unknown5 == r.unknown5);
    }

    uint32_t Packet402::getGreaseTimer() const
    {
      return swapEndian(this->greasetime) / 60;
    }

    bool Packet402::getRecirculate() const
    {
      return flag0;
    }

    std::string Packet402::getVersion() const
    {
      std::stringstream vStream;
      vStream << major << '.' << minor << '.' << patch;
      return vStream.str();
    }

    void Packet402::diff(const Packet402 *r) const
    {
      if ((this->flag0 != r->flag0) || (this->flag1 != r->flag1) || (this->flag2 != r->flag2) || (this->flag3 != r->flag3) || (this->flag4 != r->flag4) || (this->flag5 != r->flag5) || (this->flag6 != r->flag6) || (this->flag7 != r->flag7))
        ESP_LOGI(STATUSSTAG, "402 flags: %d,%d,%d,%d,%d,%d,%d,%d;", r->flag0, r->flag1, r->flag2, r->flag3, r->flag4, r->flag5, r->flag6, r->flag7);
      if (this->getGreaseTimer() != r->getGreaseTimer())
        ESP_LOGI(STATUSSTAG, "GreaseTimer: %d", r->getGreaseTimer());
      if (this->getRecirculate() != r->getRecirculate())
        ESP_LOGI(STATUSSTAG, "Recirculate: %d", r->getRecirculate());
      if (this->getVersion() != r->getVersion())
        ESP_LOGI(STATUSSTAG, "Version: %s", r->getVersion());
      if (this->major != r->major)
        ESP_LOGI(STATUSSTAG, "major 0x%02X (%d)", r->major, r->major);
      if (this->minor != r->minor)
        ESP_LOGI(STATUSSTAG, "minor 0x%02X (%d)", r->minor, r->minor);
      if (this->patch != r->patch)
        ESP_LOGI(STATUSSTAG, "patch 0x%02X (%d)", r->patch, r->patch);

      if (this->unknown1 != r->unknown1)
        ESP_LOGI(STATUSSTAG, "402 unknown1 0x%04X (%d)", r->unknown1, r->unknown1);
      if (this->unknown2 != r->unknown2)
        ESP_LOGI(STATUSSTAG, "402 unknown2 0x%02X (%d)", r->unknown2, r->unknown2);
      if (this->unknown3 != r->unknown3)
        ESP_LOGI(STATUSSTAG, "402 unknown3 0x%02X (%d)", r->unknown3, r->unknown3);
      if (this->unknown4 != r->unknown4)
        ESP_LOGI(STATUSSTAG, "402 unknown4 0x%04X (%d)", r->unknown4, r->unknown4);
      if (this->unknown5 != r->unknown5)
        ESP_LOGI(STATUSSTAG, "402 unknown5 0x%08X (%d)", r->unknown5, r->unknown5);
#if 1
      if (r->unknown1 != 0x6419)
        ESP_LOGI(STATUSSTAG, "402 unknown1 0x%04X (%d)", r->unknown1, r->unknown1);
      if (r->unknown2 != 0x0ff)
        ESP_LOGI(STATUSSTAG, "402 unknown2 0x%02X (%d)", r->unknown2, r->unknown2);
      if (r->unknown3 != 0x00)
        ESP_LOGI(STATUSSTAG, "402 unknown3 0x%02X (%d)", r->unknown3, r->unknown3);
      if (r->unknown4 != 0x0000)
        ESP_LOGI(STATUSSTAG, "402 unknown4 0x%04X (%d)", r->unknown4, r->unknown4);
      if (r->unknown5 != 0x0000)
        ESP_LOGI(STATUSSTAG, "402 unknown5 0x%04X (%d)", r->unknown5, r->unknown5);
#endif
    }

    bool operator==(const Packet402 &l, const Packet402 &r)
    {
      return (l.unknown1 == r.unknown1) && (l.unknown2 == r.unknown2) &&
             (l.flag0 == r.flag0) && (l.flag1 == r.flag1) && (l.flag2 == r.flag2) && (l.flag3 == r.flag3) && (l.flag4 == r.flag4) && (l.flag5 == r.flag5) && (l.flag6 == r.flag6) && (l.flag7 == r.flag7) &&
             (l.greasetime == r.greasetime) && (l.unknown3 == r.unknown3) && (l.major == r.major) && (l.minor == r.minor) && (l.patch == r.patch) && (l.unknown4 == r.unknown4) && (l.unknown5 == r.unknown5);
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
      return swapEndian(this->fantimer) / 60;
    }

    uint8_t Packet403::getLastFanSpeed() const
    {
      return lastfanspeed;
    }

    void Packet403::diff(const Packet403 *r) const
    {
      if (this->getAnotherTimer() != r->getAnotherTimer())
        ESP_LOGI(STATUSSTAG, "AnotherTimer: %d", r->getAnotherTimer());
      if (this->getRecirculateTimer() != r->getRecirculateTimer())
        ESP_LOGI(STATUSSTAG, "RecirculateTimer: %d", r->getRecirculateTimer());
      if (this->getFanTimer() != r->getFanTimer())
        ESP_LOGI(STATUSSTAG, "FanTimer: %d", r->getFanTimer());
      if (this->getLastFanSpeed() != r->getLastFanSpeed())
        ESP_LOGI(STATUSSTAG, "LastFanSpeed %d", r->getLastFanSpeed());

      if (this->unknown1 != r->unknown1)
        ESP_LOGI(STATUSSTAG, "403 unknown1 0x%04X (%d)", r->unknown1, r->unknown1);
      if (this->unknown3 != r->unknown3)
        ESP_LOGI(STATUSSTAG, "403 unknown3 0x%02X (%d)", r->unknown3, r->unknown3);
      if (this->unknown4 != r->unknown4)
        ESP_LOGI(STATUSSTAG, "403 unknown4 0x%08X (%d)", r->unknown4, r->unknown4);
#if 1
      if (r->unknown1 != 0x0019)
        ESP_LOGI(STATUSSTAG, "403 unknown1 0x%04X (%d)", r->unknown1, r->unknown1);
      if (r->unknown3 != 0x8D)
        ESP_LOGI(STATUSSTAG, "403 unknown3 0x%02X (%d)", r->unknown3, r->unknown3);
      if (r->unknown4 != 0x19FF6C16)
        ESP_LOGI(STATUSSTAG, "403 unknown5 0x%08X (%d)", r->unknown4, r->unknown4);
#endif
    }
    bool operator==(const Packet403 &l, const Packet403 &r)
    {
      return (l.unknown1 == r.unknown1); // todo
    }

    uint32_t Packet404::getLedTimer() const
    {
      return swapEndian(this->ledtimer) / 60;
    }

    void Packet404::diff(const Packet404 *r) const
    {
      if (this->getLedTimer() != r->getLedTimer())
        ESP_LOGI(STATUSSTAG, "LedTimer: %d", r->getLedTimer());
      if (this->unknown1 != r->unknown1)
        ESP_LOGI(STATUSSTAG, "404 unknown1 0x%08X (%d)", r->unknown1, r->unknown1);
      if (this->unknown2 != r->unknown2)
        ESP_LOGI(STATUSSTAG, "404 unknown2 0x%08X (%d)", r->unknown2, r->unknown2);
      if (this->unknown3 != r->unknown3)
        ESP_LOGI(STATUSSTAG, "404 unknown3 0x%08X (%d)", r->unknown3, r->unknown3);
      if (this->unknown4 != r->unknown4)
        ESP_LOGI(STATUSSTAG, "404 unknown4 0x%04X (%d)", r->unknown4, r->unknown4);
      if (this->unknown5 != r->unknown5)
        ESP_LOGI(STATUSSTAG, "404 unknown5 0x%02X (%d)", r->unknown5, r->unknown5);
      if (this->unknown6 != r->unknown6)
        ESP_LOGI(STATUSSTAG, "404 unknown6 0x%04X (%d)", r->unknown6, r->unknown6);
#if 1
      if (r->unknown1 != 0x00000000)
        ESP_LOGI(STATUSSTAG, "404 unknown1 0x%08X (%d)", r->unknown1, r->unknown1);
      if (r->unknown2 != 0x1901FF00)
        ESP_LOGI(STATUSSTAG, "404 unknown2 0x%08X (%d)", r->unknown2, r->unknown2);
      if (r->unknown3 != 0x00000040)
        ESP_LOGI(STATUSSTAG, "404 unknown3 0x%08X (%d)", r->unknown3, r->unknown3);
      if (r->unknown4 != 0x0000)
        ESP_LOGI(STATUSSTAG, "404 unknown4 0x%04X (%d)", r->unknown4, r->unknown4);
      if (r->unknown5 != 0x00)
        ESP_LOGI(STATUSSTAG, "404 unknown5 0x%02X (%d)", r->unknown5, r->unknown5);
      if (r->unknown6 != 0x0000)
        ESP_LOGI(STATUSSTAG, "404 unknown6 0x%04X (%d)", r->unknown6, r->unknown6);
#endif
    }
    bool operator==(const Packet404 &l, const Packet404 &r)
    {
      return (l.unknown1 == r.unknown1) && (l.unknown2 == r.unknown2); // todo
    }

  } // namespace purelinepro
} // namespace esphome
