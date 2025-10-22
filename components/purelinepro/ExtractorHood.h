#pragma once

#include <iostream>
#include <sstream>

namespace esphome
{
  namespace purelinepro
  {
    union Flags{
      struct {
          unsigned char flag0 : 1;
          unsigned char flag1 : 1;
          unsigned char flag2 : 1;
          unsigned char flag3 : 1;
          unsigned char flag4 : 1;
          unsigned char flag5 : 1;
          unsigned char flag6 : 1;
          unsigned char flag7 : 1;
      };
      uint8_t raw;
   };

    class __attribute__((__packed__)) Packet
    {
    public:
      auto operator<=>(const Packet &) const = default;

    public:
      // speed at 100% for max 5 minutes
      bool getBoost() const;
      // shutdown sequence for 10 minutes
      bool getStopping() const;
      // grease filter needs cleaning
      bool getCleanGreaseFilter() const;

      uint16_t getTimer() const;

      // light on or off
      bool getLightState() const;
      uint8_t getBrightness() const;
      uint8_t getColorTemp() const;
      uint8_t getLightMode() const;

      // fan on or off
      bool getFanState() const;
      // fan speed
      uint8_t getFanSpeed() const;

      void print() const;
      
      // print the differences between both packets
      void diff(const Packet *r) const;

    protected:
      // 0
      Flags flags1;
      uint8_t fanspeed;        // in %
      Flags flags2;
      uint8_t unknown1;
      uint8_t unknown2;
      uint8_t lightmode; // 0,1,2
      uint8_t brightness;
      uint8_t colortemp;
      uint16_t countDown;
      uint16_t unknown3;
      uint16_t unknown4;
      uint16_t unknown5;
    };

    class __attribute__((__packed__)) Packet402
    {
    public:
      auto operator<=>(const Packet402 &) const = default;

    public:
      // minutes before grease filter must be cleaned
      uint32_t getGreaseTimer() const;
      // are we in recirculate mode
      bool getRecirculate() const;

      void print() const;
      
      // print the differences between both packets, debug purposes
      void diff(const Packet402 *r) const;

    private:
      uint16_t unknown1;
      Flags flags;
      uint8_t unknown2; // ff
      uint32_t greasetime;
      uint8_t major;
      uint8_t minor;
      uint8_t patch;
      uint8_t unknown3;
      uint32_t unknown4;
      uint32_t unknown5;
    };

    class __attribute__((__packed__)) Packet403
    {
    public:
      auto operator<=>(const Packet403 &) const = default;

    public:
      uint8_t getSwitchOffFanSpeed() const; // not sure, could be unknown2 also?
      // minutes the leds have been on
      uint32_t getAnotherTimer() const;
      uint32_t getRecirculateTimer() const;
      uint32_t getFanTimer() const;
      uint8_t getFanSpeed() const; // the stored default
      uint8_t getFunctionalBrightness() const;// the stored default
      uint8_t getFunctionalColorTemp() const;// the stored default
      uint8_t getAmbiBrightness() const;// the stored default
      uint8_t getAmbiColorTemp() const;// the stored default

      uint8_t getBrightness(bool ambi) const;// the stored default
      uint8_t getColorTemp(bool ambi) const;// the stored default

      void print() const;
      
      // print the differences between both packets, debug purposes
      void diff(const Packet403 *r) const;
    private:
      uint8_t switchOffFanSpeed; //not sure
      uint8_t unknown1;
      uint32_t anotherTimer;     
      uint32_t recirculateTimer; 
      uint32_t fanTimer;
      uint8_t fanSpeed;
      uint8_t functionalBrightness;
      uint8_t functionalColor;
      uint8_t ambiBrightness;
      uint8_t ambiColor;
      uint8_t unknown2;
    };

    class __attribute__((__packed__)) Packet404
    {
    public:
      auto operator<=>(const Packet404 &) const = default;

    public:
      // minutes the fan has been on
      uint32_t getLedTimer() const;

      void print() const;
      
      // print the differences between both packets, debug purposes
      void diff(const Packet404 *r) const;

    private:
      uint32_t unknown1;
      uint32_t unknown2;
      uint32_t unknown3;
      uint8_t unknown4;
      uint32_t ledtimer;
      uint8_t unknown5;
      uint16_t unknown6;
    };

    uint16_t swapEndian(uint16_t value);
    uint32_t swapEndian(uint32_t value);

  } // namespace purelinepro
} // namespace esphome
