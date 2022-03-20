// Test with (USB uploader connected):
//   pio test -e miniultra8 -f low_power_led_backpack
// then disconnect and power with 5V on Vin with an inline multimeter

#include <Adafruit_GFX.h>
#include <Adafruit_LEDBackpack.h>
#include <LowPower.h>

Adafruit_BicolorMatrix matrix = Adafruit_BicolorMatrix();

void setup() {
  // Go to sleep to measure current before starting the display for the first time
  LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_ON);
  // RocketScream Mini-Ultra 8MHz powered at 5.2V --> 32uA measured in powerdown
}

void loop() {
  static unsigned int counter = 0;

  // Bring up the display
  matrix.begin(0x74);  // pass in the address of my own bi-color 8x8 matrix

  matrix.clear();
  matrix.setBrightness(2);  // LEDs very faded to not exceed the 5mA scale of the multimeter ;-)
  matrix.drawPixel(3, 4, LED_RED);
  matrix.drawPixel(4, 3, LED_GREEN);
  matrix.writeDisplay();
  delay(8000);

  // Power down display
  if (counter % 2 == 0) {
    // Put the display to standby mode --> measured 32uA of consumption after powerdown :-)
    matrix.end();
  } else {
    // Do the best we can to minimize power --> measured 131uA after powerdown :-(
    matrix.setBrightness(0);
    matrix.clear();
    matrix.writeDisplay();
  }

  // Instruct to terminate I2C comms
  Wire.end();

  // Reconfigure I2C pins as inputs
  pinMode(A4, INPUT_PULLUP);
  pinMode(A5, INPUT_PULLUP);

  // Go to sleep
  LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_ON);
  // under 5.2V input: 32uA after putting HT16K33 in standby, 131uA otherwise !

  // Awake again :-)
  counter++;
}
