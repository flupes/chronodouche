#include "Display.h"

#include "digits.h"
#include "utils.h"

void Display::Start() {
  matrix_.begin(0x74);  // pass in the address
  matrix_.setRotation(0);
  matrix_.setBrightness(15);
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
  anim_elapsed_ = millis();
  start_ms_ = anim_elapsed_;
}

uint32_t Display::Update(uint32_t now) {
  if ((now - anim_elapsed_) > anim_period_ms_) {
    left_rain_ = RotateRight(left_rain_, 1);
    right_rain_ = RotateRight(right_rain_, 1);
    anim_elapsed_ = now;
  }
  ldiv_t qr = ldiv(now - start_ms_, digit_period_ms_);
  digit_ = qr.quot;
  bbar_ = qr.rem / bbar_period_ms_;
  matrix_.clear();
  uint8_t digit_color = LED_YELLOW;
  if (digit_ > 9) {
    digit_color = LED_RED;
  }
  uint8_t rain_color = LED_GREEN;
  if (digit_ > 19) {
    rain_color = LED_YELLOW;
  }
  matrix_.drawBitmap(0, 0, DIGITS[digit_ % 10], 8, 8, digit_color);
  for (uint8_t b = 0; b < 8; b++) {
    const uint8_t mask = 1 << b;
    if (kBottomBar[bbar_ % kBottomBarLen] & mask) {
      matrix_.drawPixel(7 - b, 7, rain_color);
    }
    if (left_rain_ & mask) {
      matrix_.drawPixel(0, 7 - b, rain_color);
    }
    if (right_rain_ & mask) {
      matrix_.drawPixel(7, 7 - b, rain_color);
    }
  }
  matrix_.writeDisplay();
  return digit_;
}