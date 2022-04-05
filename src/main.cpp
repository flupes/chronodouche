#include <LowPower.h>

#include "Display.h"

const uint32_t kAnimIntervalMs = 150;
const uint32_t kDigitIntervalMs = 10l * 1000l;  // int is 16 bit only on the ATmega328p so
                                                // we need to qualify with long to do the math!
const uint32_t kBbarIntervalMs = kDigitIntervalMs / kBottomBarLen;
const uint32_t kNoMotionDelayMs = 5l * 1000l;
const uint32_t kPeriodBeforeSleepMs = kNoMotionDelayMs + 5l * 1000l;

const uint8_t kBuiltinLed = 13;
const uint8_t kPirOutputPin = 2;

Display gDisplay(kAnimIntervalMs, kBbarIntervalMs, kDigitIntervalMs);

enum class State { STANDBY, ACTIVE, FROZEN };

State UpdateState(State state, bool motion, uint32_t now) {
  static uint32_t last_motion = 0;

  if (motion) {
    last_motion = now;
    state = State::ACTIVE;
  } else {
    switch (state) {
      case State::ACTIVE:
        if ((now - last_motion) > kNoMotionDelayMs) {
          state = State::FROZEN;
        }
        break;
      case State::FROZEN:
        if ((now - last_motion) > kPeriodBeforeSleepMs) {
          state = State::STANDBY;
        }
        break;
      default:
        // in STANDBY and no motion, we should go to sleep!
        break;
    }
  }
  return state;
}

void MotionDetected() { detachInterrupt(digitalPinToInterrupt(kPirOutputPin)); }

void setup() {
  pinMode(kBuiltinLed, OUTPUT);
  for (uint8_t i = 0; i < 12; i++) {
    digitalWrite(kBuiltinLed, HIGH);
    delay(150);
    digitalWrite(kBuiltinLed, LOW);
    delay(100);
  }
  pinMode(kPirOutputPin, INPUT);
}

// #define DEBUG

void loop() {
  // digitalWrite(kBuiltinLed, HIGH);
  // delay(250);
  // digitalWrite(kBuiltinLed, LOW);

#ifdef DEBUG
  Serial.begin(9600);
  Serial.println("START");
  Serial.print("pin=");
  Serial.println(digitalRead(kPirOutputPin));
#endif

  gDisplay.Start();
  State state = State::ACTIVE;
  uint32_t last_wakeup = millis();
  while (state != State::STANDBY) {
    uint32_t now = millis();
    int input = digitalRead(kPirOutputPin);
    if (input == LOW) {
      digitalWrite(kBuiltinLed, HIGH);
    } else {
      if (((now - last_wakeup) / 250) % 2 == 0) {
        digitalWrite(kBuiltinLed, HIGH);
      } else {
        digitalWrite(kBuiltinLed, LOW);
      }
    }
#ifdef DEBUG
    State last_state = state;
#endif
    state = UpdateState(state, (LOW == input), now);
#ifdef DEBUG
    if (state != last_state) {
      Serial.print("new state=");
      Serial.println((uint8_t)state);
    }
#endif
    if (state == State::ACTIVE) {
      // gDisplay.Update(now);
    }
  }

#ifdef DEBUG
  Serial.println("SLEEP");
  Serial.print("pin=");
  Serial.println(digitalRead(kPirOutputPin));
  Serial.end();
#endif

  // Go to sleep
  gDisplay.Stop();
  digitalWrite(kBuiltinLed, LOW);
  // Motion with generate the pin to go LOW
  attachInterrupt(digitalPinToInterrupt(kPirOutputPin), MotionDetected, LOW);
  LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF);
}
