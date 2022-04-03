#include <Adafruit_GFX.h>
#include <Adafruit_LEDBackpack.h>
#include <LowPower.h>

#include "digits.h"
#include "utils.h"

Adafruit_BicolorMatrix matrix = Adafruit_BicolorMatrix();

const uint8_t BOTTOM_BAR[] = {0x0, 0x40, 0x42, 0x62, 0x66, 0x76, 0x7e};
const size_t BOTTOM_BAR_LEN = sizeof(BOTTOM_BAR);

const size_t k8sSleepCycles = 3;
const uint32_t kAnimIntervalMs = 150;
const uint32_t kDigitIntervalMs = 60l * 1000l;  // int is 16 bit only on the ATmega328p so
                                                // we need to qualify with long to do the math!
const uint32_t kBbarIntervalMs = kDigitIntervalMs / BOTTOM_BAR_LEN;
const uint32_t kPirStabilizationMs = 8l * 1000l;
const uint32_t kPeriodBeforeSleepMs = kPirStabilizationMs + 20l * 1000l;

const uint8_t kBuiltinLed = 13;
const uint8_t kPirPowerPin = 11;
const uint8_t kPirOutputPin = 10;

void StartPir() {
  // Provide power to the PIR (<1ma)
  digitalWrite(kPirPowerPin, HIGH);

  const uint32_t cycles = 5;
  const uint32_t onPeriod = 25;
  const uint32_t offPeriod = (kPirStabilizationMs - cycles * onPeriod) / cycles;
  // Wait enough to get a stable reading
  for (uint8_t c = 0; c < cycles; c++) {
    digitalWrite(kBuiltinLed, HIGH);
    delay(onPeriod);
    digitalWrite(kBuiltinLed, LOW);
    delay(offPeriod);
  }
  // Serial.println("PIR ON");
}

void StopPir() { digitalWrite(kPirPowerPin, LOW); }

void StartDisplay() {
  matrix.begin(0x74);  // pass in the address
  matrix.setRotation(0);
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
  static bool firstCall = true;
  static uint8_t left_rain = 0xb0;
  static uint8_t right_rain = 0x0b;
  static int bbar = 0;
  static uint32_t animElapsed = 0;
  static uint32_t bbarElapsed = 0;
  static uint32_t digitElapsed = 0;

  uint32_t now = millis();
  if (firstCall) {
    animElapsed = now;
    bbarElapsed = now;
    digitElapsed = now;
    firstCall = false;
  }

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
          StopDisplay();
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

  digitalWrite(kBuiltinLed, HIGH);
  delay(250);
  digitalWrite(kBuiltinLed, LOW);

  // Serial.begin(9600);
  // Serial.println(counter);

  if (counter % k8sSleepCycles == 0) {
    uint32_t lastWakeup = 0;
    int minutes = 0;
    State state = State::STANDBY;
    Configure();
    // Only check movement every n 8s cycles
    StartPir();
    lastWakeup = millis();
    while (true) {
      uint8_t input = digitalRead(kPirOutputPin);
      state = UpdateState(state, (HIGH == input), lastWakeup);
      if (state == State::ACTIVE) {
        minutes = UpdateDisplay(lastWakeup, minutes);
      } else {
        if (state == State::STANDBY) {
          break;
        }
      }
      delay(10);  // no real reason but to not hammer the display?
    }
    StopPir();
  }  // skip wakeup periods

  // Serial.println("SLEEP");
  // Serial.end();
  // Go to sleep
  LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_ON);
  counter++;
}
