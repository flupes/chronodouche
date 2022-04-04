#include <LowPower.h>

#include "Display.h"

const size_t k8sSleepCycles = 3;
const uint32_t kAnimIntervalMs = 150;
const uint32_t kDigitIntervalMs = 10l * 1000l;  // int is 16 bit only on the ATmega328p so
                                                // we need to qualify with long to do the math!
const uint32_t kBbarIntervalMs = kDigitIntervalMs / kBottomBarLen;
const uint32_t kPirStabilizationMs = 4l * 1000l;
const uint32_t kPeriodBeforeSleepMs = kPirStabilizationMs + 8l * 1000l;

const uint32_t kMotionDebounceCount = 3;

const uint8_t kBuiltinLed = 13;
const uint8_t kPirPowerPin = 11;
const uint8_t kPirOutputPin = 5;

Display gDisplay(kAnimIntervalMs, kBbarIntervalMs, kDigitIntervalMs);

void StartPir() {
  // Provide power to the PIR (<1ma)
  digitalWrite(kPirPowerPin, HIGH);

  const uint32_t cycles = 6;
  const uint32_t on_period = 25;
  const uint32_t off_period = (kPirStabilizationMs - cycles * on_period) / cycles;
  // Wait enough to get a stable reading
  for (uint8_t c = 0; c < cycles; c++) {
    delay(off_period);
    // digitalWrite(kBuiltinLed, HIGH);
    // mess up the sensing?
    delay(on_period);
    digitalWrite(kBuiltinLed, LOW);
  }
  Serial.println("PIR ON");
}

void StopPir() { digitalWrite(kPirPowerPin, LOW); }

enum class State { STANDBY, WAITING, ACTIVE, FROZEN };

State UpdateState(State state, bool motion, uint32_t start, uint32_t now) {
  static uint32_t last_motion = 0;
  static uint32_t motion_counter = 0;

  switch (state) {
    case State::STANDBY:
      state = State::WAITING;
      break;
    case State::WAITING:
      if (motion) {
        motion_counter++;
        if (motion_counter > kMotionDebounceCount) {
          gDisplay.Start();
          state = State::ACTIVE;
        }
      } else {
        motion_counter = 0;
        if ((now - start) > kPirStabilizationMs*2) {
          state = State::STANDBY;
        }
      }
      break;
    case State::ACTIVE:
      if (motion) {
        last_motion = now;
      } else {
        motion_counter = 0;
        if ((now - last_motion) > kPirStabilizationMs) {
          state = State::FROZEN;
        }
      }
      break;
    case State::FROZEN:
      if (motion) {
        motion_counter++;
        if (motion_counter > kMotionDebounceCount) {
          state = State::ACTIVE;
        }
      } else {
        motion_counter = 0;
        if ((now - last_motion) > kPeriodBeforeSleepMs) {
          state = State::STANDBY;
          gDisplay.Stop();
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
  pinMode(kPirOutputPin, INPUT);
}

void setup() {
  // nothing (need to reconfigure at each wakeup)
}

void loop() {
  static uint32_t counter = 0;

  digitalWrite(kBuiltinLed, HIGH);
  delay(250);
  digitalWrite(kBuiltinLed, LOW);

  Serial.begin(9600);
  Serial.println(counter);

  if (counter % k8sSleepCycles == 0) {
    Serial.println("START");
    uint32_t last_wakeup = 0;
    State state = State::STANDBY;
    Configure();
    // Only check movement every n 8s cycles
    StartPir();
    last_wakeup = millis();
    uint8_t last_input = 0;
    while (true) {
      uint8_t input = digitalRead(kPirOutputPin);
      uint32_t now = millis();
      if (last_input != input) {
        Serial.print("input=");
        Serial.println(input);
        last_input = input;
      }
      State last_state = state;
      state = UpdateState(state, (HIGH == input), last_wakeup, now);
      if (state != last_state) {
        Serial.print("new state=");
        Serial.println((uint8_t)state);
      }
      if (state == State::ACTIVE) {
        gDisplay.Update(last_wakeup, now);
      } else {
        if (state == State::STANDBY) {
          // if ((now - last_wakeup) > 2000) {
          break;
          // }
        }
      }
      delay(50);  // no real reason but to not hammer the display?
    }
    StopPir();
  }  // skip wakeup periods

  Serial.println("SLEEP");
  Serial.end();
  // Go to sleep
  LowPower.powerDown(SLEEP_2S, ADC_OFF, BOD_ON);
  counter++;
}
