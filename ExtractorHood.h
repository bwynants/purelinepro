#pragma once

#include <iostream>
#include <sstream>

#include "BLEDevice.h"

namespace esphome
{
  namespace purelinepro
  {

    extern BLEUUID Serv_ExtractorHood; // the service

    struct Packet
    {
      // 0
      unsigned char flag0: 1; // 
      unsigned char timerflag: 1; // timer running
      unsigned char flag2: 1; //
      unsigned char flag3: 1;
      unsigned char flag4: 1;
      unsigned char flag5: 1;
      unsigned char flag6: 1;
      unsigned char flag7: 1;
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
      uint16_t unknown4;
      // 6
      uint16_t unknown5;
      // 7
      uint16_t unknown6;
    };
    inline bool operator==(const Packet &l, const Packet &r)
    {
      return (l.fanspeed == r.fanspeed) && (l.lightmode == r.lightmode) && (l.brightness == r.brightness) && (l.colortemp == r.colortemp) && (l.countDown == r.countDown) &&
             (l.flag0 == r.flag0) && (l.timerflag == r.timerflag) && (l.flag2 == r.flag2) && (l.flag3 == r.flag3) && (l.flag4 == r.flag4) && (l.flag5 == r.flag5) && (l.flag6 == r.flag6) && (l.flag7 == r.flag7) &&
             (l.unknown1 == r.unknown1) && (l.unknown2 == r.unknown2) && (l.unknown4 == r.unknown4) && (l.unknown5 == r.unknown5) && (l.unknown6 == r.unknown6);
    }
    inline bool operator!=(const Packet &lhs, const Packet &rhs) { return !(lhs == rhs); }

    void parseAndPrintFields(const uint8_t *data, size_t len);
    void printServices(BLEAdvertisedDevice *advertisedDevice);
    void printServices(BLEClient *pClient);

  } // namespace purelinepro
} // namespace esphome
