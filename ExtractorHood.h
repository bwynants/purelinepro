#pragma once

#include <iostream>
#include <sstream>

namespace esphome
{
  namespace purelinepro
  {
    class __attribute__((__packed__)) Packet
    {
    public:
      // speed at 100% for max 5 minues
      bool getBoost() const;
      // shudown sequence for 10 minutes
      bool getStopping() const;

      uint16_t getTimer() const;

      // light on or off
      bool getLightState() const;
      uint8_t getBrightness() const;
      uint8_t getColorTemp() const;

      // fan on or off0
      bool getFanState() const;
      uint8_t getFanSpeed() const;

      // print the differences between both packets
      void diff(const Packet *r) const;

      // compare
      friend bool operator==(const Packet &l, const Packet &r);

    protected:
      // 0
      unsigned char flag0 : 1; //
      unsigned char flag1 : 1; // timer running
      unsigned char flag2 : 1; //
      unsigned char flag3 : 1;
      unsigned char flag4 : 1;
      unsigned char flag5 : 1;
      unsigned char flag6 : 1;
      unsigned char flag7 : 1;
      uint8_t fanspeed; // in %
      uint16_t unknown1;
      uint8_t unknown2;
      uint8_t lightmode; // 0,1,2
      uint8_t brightness;
      uint8_t colortemp;
      uint16_t countDown;
      uint16_t unknown3;
      uint16_t unknown4;
      uint16_t unknown5;
    };

    inline bool operator!=(const Packet &lhs, const Packet &rhs) { return !(lhs == rhs); }

    class __attribute__((__packed__)) Packet402
    {
    public:
      // minutes before grease filter must be cleaned
      uint32_t getGreaseTimer() const;
      // are we in re-circukate moe
      bool getRecirculate() const;
      // version of the firmware
      std::string getVersion() const;

      // print the differences between both packets
      void diff(const Packet402 *r) const;

      // compare
      friend bool operator==(const Packet402 &l, const Packet402 &r);

    private:
      uint16_t unknown1;
      unsigned char flag0 : 1;
      unsigned char flag1 : 1;
      unsigned char flag2 : 1;
      unsigned char flag3 : 1;
      unsigned char flag4 : 1;
      unsigned char flag5 : 1;
      unsigned char flag6 : 1;
      unsigned char flag7 : 1;
      uint8_t unknown2; // ff
      uint32_t greasetime;
      uint8_t major;
      uint8_t minor;
      uint8_t patch;
      uint8_t unknown3;
      uint32_t unknown4;
      uint32_t unknown5;
    };

    inline bool operator!=(const Packet402 &lhs, const Packet402 &rhs) { return !(lhs == rhs); }

    class __attribute__((__packed__)) Packet403
    {
    public:
      // minutes the leds have been on
      uint32_t getFanTimer() const;
      uint32_t getLastFanSpeed() const;

      // print the differences between both packets
      void diff(const Packet403 *r) const;

      // compare
      friend bool operator==(const Packet403 &l, const Packet403 &r);

    private:
      uint32_t unknown1;
      uint32_t unknown2;
      uint16_t unknown3;
      uint32_t fantimer;
      uint8_t lastfanspeed;
      uint8_t unknown5;
      uint32_t unknown6;
    };

    inline bool operator!=(const Packet403 &lhs, const Packet403 &rhs) { return !(lhs == rhs); }

    class __attribute__((__packed__)) Packet404
    {
    public:
      // minutes the fan has been on
      uint32_t getLedTimer() const;

      // print the differences between both packets
      void diff(const Packet404 *r) const;

      // compare
      friend bool operator==(const Packet404 &l, const Packet404 &r);

    private:
      uint32_t unknown1;
      uint32_t unknown2;
      uint32_t unknown3;
      uint8_t unknown4;
      uint32_t ledtimer;
      uint8_t unknown5;
      uint16_t unknown6;
    };

    inline bool operator!=(const Packet404 &lhs, const Packet404 &rhs) { return !(lhs == rhs); }

    uint16_t swapEndian(uint16_t value);
    uint32_t swapEndian(uint32_t value);

  } // namespace purelinepro
} // namespace esphome
