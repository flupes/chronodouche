#include "Display.h"

#include "digits.h"
#include "utils.h"

void Display::Start() {
  matrix_.begin(0x74);  // pass in the address
  matrix_.setRotation(0);
  matrix_.setBrightness(15);
  anim_elapsed_ = millis();
  bbard_elapsed_ = anim_elapsed_;
  digit_elapsed_ = anim_elapsed_;
  Reset();
}

void Display::Stop() {
  matrix_.setBrightness(0);
  matrix_.clear();
  matrix_.writeDisplay();
  matrix_.end();
  Wire.end();
  pinMode(A4, INPUT_PULLUP);
  pinMode(A5, INPUT_PULLUP);
}

void Display::Reset() {
  left_rain_ = kLeftRain;
  right_rain_ = kRightRain;
  bbar_ = 0;
  digit_ = 0;
}

uint32_t Display::Update(uint32_t start, uint32_t now) {
  if ((now - anim_elapsed_) > anim_period_ms_) {
    left_rain_ = RotateRight(left_rain_, 1);
    right_rain_ = RotateRight(right_rain_, 1);
    anim_elapsed_ = now;
  }
  if ((now - bbard_elapsed_) > bbar_period_ms_) {
    bbar_ = (bbar_ + 1) % kBottomBarLen;
    bbard_elapsed_ = now;
  }
  if ((now - digit_elapsed_) > digit_period_ms_) {
    digit_++;
    digit_elapsed_ = now;
  }
  matrix_.clear();
  uint8_t color = LED_YELLOW;
  if (digit_ > 9) {
    color = LED_RED;
  }
  matrix_.drawBitmap(0, 0, DIGITS[digit_ % 10], 8, 8, color);
  for (uint8_t b = 0; b < 8; b++) {
    const uint8_t mask = 1 << b;
    if (kBottomBar[bbar_] & mask) {
      matrix_.drawPixel(7 - b, 7, LED_GREEN);
    }
    if (left_rain_ & mask) {
      matrix_.drawPixel(0, 7 - b, LED_GREEN);
    }
    if (right_rain_ & mask) {
      matrix_.drawPixel(7, 7 - b, LED_GREEN);
    }
  }
  matrix_.writeDisplay();
  return digit_;
}