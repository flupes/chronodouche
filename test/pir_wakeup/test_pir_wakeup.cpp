#include <Arduino.h>
#include <LowPower.h>
#include <stdint.h>

// The ATmega328P can only detect external interrupts when the pin state is LOW
// in powerdown mode (no oscillator)
// So the circuit use a NOT gate to transform the motion==HIGH Adafruit PIR
// to a motion==LOW for this application

const uint8_t kPirOutputPin = 2;

const uint8_t kBuiltinLed = 13;

static volatile bool gMotion;
static volatile bool gChange;

void MotionDetected() { detachInterrupt(digitalPinToInterrupt(kPirOutputPin)); }

void setup() {
  pinMode(kBuiltinLed, OUTPUT);
  for (uint8_t i=0; i<16; i++) {
    digitalWrite(kBuiltinLed, HIGH);
    delay(200);
    digitalWrite(kBuiltinLed, LOW);
    delay(50);
  }

  pinMode(kPirOutputPin, INPUT);
}

void loop() {
  digitalWrite(kBuiltinLed, HIGH);

  while (true) {
    digitalWrite(kBuiltinLed, HIGH);
    delay(800);
    digitalWrite(kBuiltinLed, LOW);
    delay(200);
    // When motion stopped, pin will go HIGH
    if (digitalRead(kPirOutputPin) == HIGH) break;
  }

  // Motion with generate the pin to go LOW
  attachInterrupt(digitalPinToInterrupt(kPirOutputPin), MotionDetected, LOW);
  LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF);
}
