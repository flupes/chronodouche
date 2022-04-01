#include <Adafruit_GFX.h>
#include <Adafruit_LEDBackpack.h>
#include <LowPower.h>

#include "utils.h"
#include "digits.h"

Adafruit_BicolorMatrix matrix = Adafruit_BicolorMatrix();

const uint8_t BOTTOM_BAR[] = {0x0, 0x40, 0x42, 0x62, 0x66, 0x76, 0x7e};
const uint8_t BOTTOM_BAR_LEN = sizeof(BOTTOM_BAR);

const uint32_t kAnimIntervalMs = 150;
const uint32_t kDigitIntervalMs = 6*1000;
const uint32_t kBbarIntervalMs = kDigitIntervalMs / BOTTOM_BAR_LEN;

bool Update(uint32_t start) {
  static uint8_t left_rain = 0xb0;
  static uint8_t right_rain = 0x0b;
  static int digit = 0;
  static int bbar = 0;
  static uint32_t animElapsed = millis();
  static uint32_t bbarElapsed = millis();
  static uint32_t digitElapsed = millis();

  uint32_t now = millis();
  
  if ((now - animElapsed) > kAnimIntervalMs) {
    left_rain = RotateRight(left_rain, 1);
    right_rain = RotateRight(right_rain, 1);
    animElapsed = now;
  }
  if ((now - bbarElapsed) > kBbarIntervalMs) {
    bbar = (bbar + 1) % BOTTOM_BAR_LEN;
    bbarElapsed = now;
  }
  if ((now - digitElapsed) > kDigitIntervalMs) {
    digit = (digit + 1) % 10;
    digitElapsed = now;
  }
  matrix.clear();
  matrix.drawBitmap(0, 0, DIGITS[digit], 8, 8, LED_YELLOW);
  for (uint8_t b = 0; b < 8; b++) {
    const uint8_t mask = 1 << b;
    if (BOTTOM_BAR[bbar] & mask) {
      matrix.drawPixel(7-b, 7, LED_GREEN);
    }
    if (left_rain & mask) {
      matrix.drawPixel(0, 7-b, LED_GREEN);
    }
    if (right_rain & mask) {
      matrix.drawPixel(7, 7-b, LED_GREEN);
    }
  }
  matrix.writeDisplay();
  if ((now - start) > 10 * kDigitIntervalMs) {
    return true;
  } else {
    return false;
  }
}

void setup() {
  // Go to sleep to measure current before starting the display for the first time
  LowPower.powerDown(SLEEP_4S, ADC_OFF, BOD_ON);
}

void loop() {
  uint32_t start = millis();

  // Bring up the display
  matrix.begin(0x74);  // pass in the address
  matrix.setRotation(3);
  matrix.setBrightness(15);

  while (!Update(start)) {
    // nothing really to do ;-)
  }

  // Power down everything
  matrix.setBrightness(0);
  matrix.clear();
  matrix.writeDisplay();
  matrix.end();
  Wire.end();
  pinMode(A4, INPUT_PULLUP);
  pinMode(A5, INPUT_PULLUP);

  // Go to slepp
  LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_ON);
}
