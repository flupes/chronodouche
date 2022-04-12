// Test with:
// pio test -e miniultra8 -f animation

#include <LowPower.h>
#include "Display.h"

const uint32_t kAnimIntervalMs = 150;
const uint32_t kDigitIntervalMs = 6*1000;

Display gDisplay(kAnimIntervalMs, kDigitIntervalMs);

void setup() {
  // Go to sleep to measure current before starting the display for the first time
  LowPower.powerDown(SLEEP_4S, ADC_OFF, BOD_ON);
}

void loop() {
  // Bring up the display
  gDisplay.Start();

  while (gDisplay.Update(millis()) < 10) {
    // nothing really to do ;-)
  }

  // Power down everything
  gDisplay.Stop();

  // Go to slepp
  LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_ON);
}
