#include <Adafruit_GFX.h>
#include <Adafruit_LEDBackpack.h>
#include <LowPower.h>

#include "digits.h"
#include "utils.h"

Adafruit_BicolorMatrix matrix = Adafruit_BicolorMatrix();

const uint8_t BOTTOM_BAR[] = {0x0, 0x40, 0x42, 0x62, 0x66, 0x76, 0x7e};
const uint8_t BOTTOM_BAR_LEN = sizeof(BOTTOM_BAR);

const uint32_t k8sSleepCycles = 1;
const uint32_t kAnimIntervalMs = 150;
const uint32_t kDigitIntervalMs = 6 * 1000;
const uint32_t kBbarIntervalMs = kDigitIntervalMs / BOTTOM_BAR_LEN;
const uint32_t kPirStabilizationMs = 8 * 1000;
const uint32_t kPeriodBeforeSleepMs = kPirStabilizationMs + 8 * 1000;

const uint8_t kPirPowerPin = 11;
const uint8_t kPirOutputPin = 10;

void StartPir() {
  // Provide power to the PIR (<1ma)
  digitalWrite(kPirPowerPin, HIGH);
  // Wait enough to get a stable reading
  delay(kPirStabilizationMs);
  Serial.println("PIR ON");
}

void StopPir() { digitalWrite(kPirPowerPin, LOW); }

void StartDisplay() {
  matrix.begin(0x74);  // pass in the address
  matrix.setRotation(3);
  matrix.setBrightness(15);
}

void StopDisplay() {
  matrix.setBrightness(0);
  matrix.clear();
  matrix.writeDisplay();
  matrix.end();
  Wire.end();
  pinMode(A4, INPUT_PULLUP);
  pinMode(A5, INPUT_PULLUP);
}

int UpdateDisplay(uint32_t start, int number) {
  static uint8_t left_rain = 0xb0;
  static uint8_t right_rain = 0x0b;
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
    number++;
    digitElapsed = now;
  }
  matrix.clear();
  uint8_t color = LED_YELLOW;
  if (number > 9) {
    color = LED_RED;
  }
  matrix.drawBitmap(0, 0, DIGITS[number % 10], 8, 8, color);
  for (uint8_t b = 0; b < 8; b++) {
    const uint8_t mask = 1 << b;
    if (BOTTOM_BAR[bbar] & mask) {
      matrix.drawPixel(7 - b, 7, LED_GREEN);
    }
    if (left_rain & mask) {
      matrix.drawPixel(0, 7 - b, LED_GREEN);
    }
    if (right_rain & mask) {
      matrix.drawPixel(7, 7 - b, LED_GREEN);
    }
  }
  matrix.writeDisplay();
  return number;
}

enum class State { STANDBY, WAITING, ACTIVE, FROZEN };

State UpdateState(State state, bool motion, uint32_t start) {
  static uint32_t lastMotion = 0;

  uint32_t now = millis();
  switch (state) {
    case State::STANDBY:
      state = State::WAITING;
      break;
    case State::WAITING:
      if (motion) {
        StartDisplay();
        state = State::ACTIVE;
      } else {
        if ((now - start) > kPirStabilizationMs) {
          state = State::STANDBY;
        }
      }
      break;
    case State::ACTIVE:
      if (motion) {
        lastMotion = now;
      } else {
        if ((now - lastMotion) > kPirStabilizationMs) {
          state = State::FROZEN;
        }
      }
      break;
    case State::FROZEN:
      if (motion) {
        state = State::ACTIVE;
      } else {
        if ((now - lastMotion) > kPeriodBeforeSleepMs) {
          state = State::STANDBY;
        }
      }
      break;
    default:
      // should not happen
      break;
  }
  return state;
}

void Configure() {
  pinMode(kPirPowerPin, OUTPUT);
  pinMode(kPirOutputPin, INPUT_PULLUP);
}

void setup() {
  // nothing (need to reconfigure at each wakeup)
}

void loop() {
  static uint32_t counter = 0;

  uint32_t lastWakeup = 0;
  int minutes = 0;
  State state = State::STANDBY;

  if (counter % k8sSleepCycles == 0) {
    Configure();

    Serial.begin(9600);
    Serial.println("WAKEUP");

    // Only check movement every n 8s cycles
    StartPir();
    lastWakeup = millis();
    while (true) {
      uint8_t input = digitalRead(kPirOutputPin);
      // Serial.print("motion=");
      // Serial.print(input);
      bool motion = (HIGH == input);
      state = UpdateState(state, motion, lastWakeup);
      if (state == State::ACTIVE) {
        minutes = UpdateDisplay(lastWakeup, minutes);
      } else if (state == State::STANDBY) {
        break;
      }
      // Serial.print(" / new state=");
      // Serial.println((uint8_t)state);
      // if (state == State::STANDBY) {
      //   break;
      // }
      delay(10);  // no real reason but to not hammer the display?
    }
    StopPir();
    StopDisplay();
    Serial.end();
  }  // skip wakeup periods

  // Go to sleep
  // delay(4000);
  LowPower.powerDown(SLEEP_4S, ADC_OFF, BOD_ON);
}
