#include "esphome.h"
#ifdef USE_TIME
#include "esphome/core/time.h"
#endif
#include "ExtractorHood.h"

namespace esphome
{
  namespace purelinepro
  {

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
      return flag1 && fanspeed > 75;
    }
    bool Packet::getStopping() const
    {
      return flag1 && fanspeed == 25;
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
#if 0
      if ((this->flag0 != r->flag0) || (this->flag1 != r->flag1) || (this->flag2 != r->flag2) || (this->flag3 != r->flag3) || (this->flag4 != r->flag4) || (this->flag5 != r->flag5) || (this->flag6 != r->flag6) || (this->flag7 != r->flag7) || (this->fanspeed != r->fanspeed) || (this->lightmode != r->lightmode) || (this->brightness != r->brightness) || (this->colortemp != r->colortemp))
        ESP_LOGI(TAG, "flags: %d,%d,%d,%d,%d,%d,%d,%d; speed: %d lightmode: %s, b:%d t:%d", r->flag0, r->flag1, r->flag2, r->flag3, r->flag4, r->flag5, r->flag6, r->flag7, r->fanspeed, lightMode[r->lightmode].c_str(), r->brightness, r->colortemp);
#endif
      if ((this->flag0 != r->flag0) || (this->flag1 != r->flag1) || (this->flag2 != r->flag2) || (this->flag3 != r->flag3) || (this->flag4 != r->flag4) || (this->flag5 != r->flag5) || (this->flag6 != r->flag6) || (this->flag7 != r->flag7))
        ESP_LOGI(TAG, "flags: %d,%d,%d,%d,%d,%d,%d,%d", r->flag0, r->flag1, r->flag2, r->flag3, r->flag4, r->flag5, r->flag6, r->flag7);
      if ((this->fanspeed != r->fanspeed) || (this->lightmode != r->lightmode) || (this->brightness != r->brightness) || (this->colortemp != r->colortemp))
        ESP_LOGI(TAG, "speed: %d lightmode: %s, b:%d t:%d", r->fanspeed, lightMode[r->lightmode].c_str(), r->brightness, r->colortemp);
      if (this->getTimer() != r->getTimer())
        ESP_LOGI(TAG, "countDown: %d", r->getTimer());
      if (this->unknown1 != r->unknown1)
        ESP_LOGI(TAG, "unknown1 0x%04X (%d)", r->unknown1, (int16_t)r->unknown1);
      if (this->unknown2 != r->unknown2)
        ESP_LOGI(TAG, "unknown2 0x%02X (%d)", r->unknown2, (int8_t)r->unknown2);
      if (this->unknown3 != r->unknown3)
        ESP_LOGI(TAG, "unknown6 0x%04X (%d)", r->unknown3, (int16_t)r->unknown3);
      if (this->unknown4 != r->unknown4)
        ESP_LOGI(TAG, "unknown4 0x%04X (%d)", r->unknown4, (int16_t)r->unknown4);
      if (this->unknown5 != r->unknown5)
        ESP_LOGI(TAG, "unknown5 0x%04X (%d)", r->unknown5, (int16_t)r->unknown5);
#if 1
      if (this->unknown1 != 0x00)
        ESP_LOGI(TAG, "unknown1 0x%04X (%d)", this->unknown1, (int16_t)this->unknown1);
      if (this->unknown2 != 0xFF)
        ESP_LOGI(TAG, "unknown2 0x%02X (%d)", this->unknown2, (int8_t)this->unknown2);
      if (this->unknown3 != 0x0100)
        ESP_LOGI(TAG, "unknown3 0x%04X (%d)", this->unknown3, (int16_t)this->unknown3);
      if (this->unknown4 != 0x00FF)
        ESP_LOGI(TAG, "unknown4 0x%04X (%d)", this->unknown4, (int16_t)this->unknown4);
      if (this->unknown5 != 0x0000)
        ESP_LOGI(TAG, "unknown5 0x%04X (%d)", this->unknown5, (int16_t)this->unknown5);
#endif
    }

    bool operator==(const Packet &l, const Packet &r)
    {
      return (l.fanspeed == r.fanspeed) && (l.lightmode == r.lightmode) && (l.brightness == r.brightness) && (l.colortemp == r.colortemp) && (l.countDown == r.countDown) &&
             (l.flag0 == r.flag0) && (l.flag1 == r.flag1) && (l.flag2 == r.flag2) && (l.flag3 == r.flag3) && (l.flag4 == r.flag4) && (l.flag5 == r.flag5) && (l.flag6 == r.flag6) && (l.flag7 == r.flag7) &&
             (l.unknown1 == r.unknown1) && (l.unknown2 == r.unknown2) && (l.unknown3 == r.unknown3) && (l.unknown4 == r.unknown4) && (l.unknown5 == r.unknown5);
    }

    uint32_t ExtraPacket::getGreaseTimer() const

    {
      return swapEndian(greasetime) / 3600;
    }

    bool ExtraPacket::getRecirculate() const
    {
      return flag7;
    }

    std::string ExtraPacket::getVersion() const
    {
      std::stringstream vStream;
      vStream << major << '.' << minor << '.' << patch;
      return vStream.str();
    }

    void ExtraPacket::diff(const ExtraPacket *r) const
    {
      if ((this->flag0 != r->flag0) || (this->flag1 != r->flag1) || (this->flag2 != r->flag2) || (this->flag3 != r->flag3) || (this->flag4 != r->flag4) || (this->flag5 != r->flag5) || (this->flag6 != r->flag6) || (this->flag7 != r->flag7))
        ESP_LOGI(TAG, "flags: %d,%d,%d,%d,%d,%d,%d,%d;", r->flag0, r->flag1, r->flag2, r->flag3, r->flag4, r->flag5, r->flag6, r->flag7);
      if (this->getGreaseTimer() != r->getGreaseTimer())
        ESP_LOGI(TAG, "GreaseTimer: %d", r->getGreaseTimer());
      if (this->getRecirculate() != r->getRecirculate())
        ESP_LOGI(TAG, "Recirculate: %d", r->getRecirculate());
      if (this->major != r->major)
        ESP_LOGI(TAG, "major 0x%02X (%d)", r->major, (int16_t)r->major);
      if (this->minor != r->minor)
        ESP_LOGI(TAG, "minor 0x%02X (%d)", r->minor, (int16_t)r->minor);
      if (this->patch != r->patch)
        ESP_LOGI(TAG, "patch 0x%02X (%d)", r->patch, (int16_t)r->patch);

      if (this->unknown1 != r->unknown1)
        ESP_LOGI(TAG, "unknown1 0x%04X (%d)", r->unknown1, (int16_t)r->unknown1);
      if (this->unknown2 != r->unknown2)
        ESP_LOGI(TAG, "unknown2 0x%02X (%d)", r->unknown2, (int8_t)r->unknown2);
      if (this->unknown3 != r->unknown3)
        ESP_LOGI(TAG, "unknown3 0x%02X (%d)", r->unknown3, (int16_t)r->unknown3);
      if (this->unknown4 != r->unknown4)
        ESP_LOGI(TAG, "unknown4 0x%04X (%d)", r->unknown4, (int16_t)r->unknown4);
      if (this->unknown5 != r->unknown5)
        ESP_LOGI(TAG, "unknown5 0x%08X (%d)", r->unknown5, (int16_t)r->unknown5);
#if 1
      if (this->unknown1 != 0x6419)
        ESP_LOGI(TAG, "unknown1 0x%04X (%d)", this->unknown1, (int16_t)this->unknown1);
      if (this->unknown2 != 0x0ff)
        ESP_LOGI(TAG, "unknown2 0x%02X (%d)", this->unknown2, (int8_t)this->unknown2);
      if (this->unknown3 != 0x00)
        ESP_LOGI(TAG, "unknown3 0x%02X (%d)", this->unknown3, (int16_t)this->unknown3);
      if (this->unknown4 != 0x0000)
        ESP_LOGI(TAG, "unknown4 0x%04X (%d)", this->unknown4, (int16_t)this->unknown4);
      if (this->unknown5 != 0x0000)
        ESP_LOGI(TAG, "unknown5 0x%04X (%d)", this->unknown5, (int16_t)this->unknown5);
#endif
    }
    bool operator==(const ExtraPacket &l, const ExtraPacket &r)
    {
      return (l.unknown1 == r.unknown1) && (l.unknown2 == r.unknown2) &&
             (l.flag0 == r.flag0) && (l.flag1 == r.flag1) && (l.flag2 == r.flag2) && (l.flag3 == r.flag3) && (l.flag4 == r.flag4) && (l.flag5 == r.flag5) && (l.flag6 == r.flag6) && (l.flag7 == r.flag7) &&
             (l.greasetime == r.greasetime) && (l.unknown3 == r.unknown3) && (l.major == r.major) && (l.minor == r.minor) && (l.patch == r.patch) && (l.unknown4 == r.unknown4) && (l.unknown5 == r.unknown5);
    }

  } // namespace purelinepro
} // namespace esphome
