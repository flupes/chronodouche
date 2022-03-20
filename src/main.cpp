#include <Adafruit_GFX.h>
#include <Adafruit_LEDBackpack.h>
#include <LowPower.h>

#include "font_ibm_bios_8x8.h"
#include "utils.h"
// #include "Fonts/Org_01.h"
// #include "Fonts/Picopixel.h"
Adafruit_BicolorMatrix matrix = Adafruit_BicolorMatrix();

const uint32_t kAnimIntervalMs = 150;
const uint32_t kDigitIntervalMs = 1000;

bool Update(uint32_t start) {
  static uint8_t left = 0x22;
  static uint8_t right = 0x88;
  static int digit = 0;
  static uint32_t animElapsed = millis();
  static uint32_t digitElapsed = millis();
  uint32_t now = millis();
  if ((now - animElapsed) > kAnimIntervalMs) {
    left = RotateLeft(left, 1);
    right = RotateLeft(right, 1);
    // left = RotateRight(left, 1);
    // right = RotateRight(right, 1);
    animElapsed = now;
  }
  if ((now - digitElapsed) > kDigitIntervalMs) {
    digit = (digit + 1) % 10;
    digitElapsed = now;
  }
  matrix.clear();
  matrix.drawBitmap(1, 0, font8x8_ibm_bios[digit + 0x10], 8, 8, LED_RED);
  for (uint8_t b = 0; b < 8; b++) {
    if (left & (1 << b)) {
      matrix.drawPixel(0, b, LED_GREEN);
    }
    if (right & (1 << b)) {
      matrix.drawPixel(7, b, LED_GREEN);
    }
  }
  // matrix.drawBitmap(0, 0, &left, 1, 8, LED_GREEN);
  // matrix.drawBitmap(7, 0, &right, 1, 8, LED_GREEN);
  matrix.writeDisplay();
  if ((now - start) > 10000) {
    return true;
  } else {
    return false;
  }
}

void setup() {
  // Go to sleep to measure current before starting the display for the first time
  LowPower.powerDown(SLEEP_4S, ADC_OFF, BOD_OFF);
}

void loop() {
  uint32_t start = millis();

  // Bring up the display
  matrix.begin(0x74);  // pass in the address
  matrix.setRotation(1);
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
  LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
}
