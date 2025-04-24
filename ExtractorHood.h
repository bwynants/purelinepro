#pragma once

#include <iostream>
#include <sstream>

namespace esphome
{
  namespace purelinepro
  {
    class Packet
    {
    public:
      // speed at 100% for max 5 minues
      bool getBoost() const;
      // shudown sequence for 10 minutes
      bool getStopping() const;

      uint16_t getTimer() const;

      bool getLightState() const;
      uint8_t getBrightness() const;
      uint8_t getColorTemp() const;

      bool getFanState() const;
      uint8_t getFanSpeed() const;

      void diff(const Packet *r) const;

      friend bool operator==(const Packet &l, const Packet &r);

    protected:
      // 0
      unsigned char flag0 : 1;     //
      unsigned char flag1 : 1; // timer running
      unsigned char flag2 : 1;     //
      unsigned char flag3 : 1;
      unsigned char flag4 : 1;
      unsigned char flag5 : 1;
      unsigned char flag6 : 1;
      unsigned char flag7 : 1;
      uint8_t fanspeed; // in %
      // 1
      uint16_t unknown1;
      // 2
      uint8_t unknown2;
      uint8_t lightmode; // 0,1,2
      // 3
      uint8_t brightness;
      uint8_t colortemp;
      // 4
      uint16_t countDown;
      // 5
      uint16_t unknown3;
      // 6
      uint16_t unknown4;
      // 7
      uint16_t unknown5;
    };

    inline bool operator!=(const Packet &lhs, const Packet &rhs) { return !(lhs == rhs); }

    class ExtraPacket
    {
    public:
      uint32_t getGreaseTimer() const;
      bool getRecirculate() const;
      std::string getVersion() const;

      void diff(const ExtraPacket *r) const;

      friend bool operator==(const ExtraPacket &l, const ExtraPacket &r);

    private:
      uint16_t unknown1;  // 1964
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
      uint8_t unknown3;
      uint8_t patch;
      uint8_t minor;
      uint32_t unknown4;
      uint32_t unknown5;
    };
    inline bool operator!=(const ExtraPacket &lhs, const ExtraPacket &rhs) { return !(lhs == rhs); }

    uint16_t swapEndian(uint16_t value);
    uint32_t swapEndian(uint32_t value);
    
  } // namespace purelinepro
} // namespace esphome
