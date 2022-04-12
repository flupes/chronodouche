#pragma once

#include <Adafruit_GFX.h>
#include <Adafruit_LEDBackpack.h>
#include <stdint.h>

const uint8_t kLeftRain = 0xb0;
const uint8_t kRightRain = 0x0b;
const uint8_t kBottomBar[] = {0x0, 0x40, 0x42, 0x62, 0x66, 0x76, 0x7e};
const size_t kBottomBarLen = sizeof(kBottomBar);

class Display {
 public:
  Display(uint32_t animPeriod, uint32_t digitPeriod)
      : anim_period_ms_(animPeriod),
        digit_period_ms_(digitPeriod),
        bbar_period_ms_(digitPeriod / kBottomBarLen),
        anim_elapsed_(0),
        digit_elapsed_(0),
        digit_(0),
        bbar_(0),
        matrix_(Adafruit_BicolorMatrix()) {}

  void Start();
  void Stop();
  void Reset();
  uint32_t Update(uint32_t now);

 protected:
  const uint32_t anim_period_ms_;
  const uint32_t digit_period_ms_;
  const uint32_t bbar_period_ms_;
  uint32_t anim_elapsed_;
  uint32_t digit_elapsed_;
  uint32_t digit_;
  uint8_t bbar_;
  uint8_t left_rain_;
  uint8_t right_rain_;
  Adafruit_BicolorMatrix matrix_;
};
