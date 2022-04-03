// Test with:
// pio test -e miniultra8 -f animation

#include <LowPower.h>
#include "Display.h"

const uint32_t kAnimIntervalMs = 150;
const uint32_t kDigitIntervalMs = 6*1000;
const uint32_t kBbarIntervalMs = kDigitIntervalMs / kBottomBarLen;

Display gDisplay(kAnimIntervalMs, kBbarIntervalMs, kDigitIntervalMs);

void setup() {
  // Go to sleep to measure current before starting the display for the first time
  LowPower.powerDown(SLEEP_4S, ADC_OFF, BOD_ON);
}

void loop() {
  uint32_t start = millis();

  // Bring up the display
  gDisplay.Start();

  while (gDisplay.Update(start) < 10) {
    // nothing really to do ;-)
  }

  // Power down everything
  gDisplay.Stop();

  // Go to slepp
  LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_ON);
}
