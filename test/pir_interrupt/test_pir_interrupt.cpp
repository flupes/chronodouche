#include <Arduino.h>
#include <stdint.h>

const uint8_t kPirOutputPin = 2;

const uint8_t kBuiltinLed = 13;

static volatile bool gMotion;
static volatile bool gChange;

void MovementDetected();

void NoMoreMotion() {
  detachInterrupt(digitalPinToInterrupt(kPirOutputPin));
  gMotion = false;
  gChange = true;
  // Serial.println("ISR STEADY");
  attachInterrupt(digitalPinToInterrupt(kPirOutputPin), MovementDetected, RISING);
}

void MovementDetected() {
  detachInterrupt(digitalPinToInterrupt(kPirOutputPin));
  gMotion = true;
  gChange = true;
  // Serial.println("ISR MOTION");
  attachInterrupt(digitalPinToInterrupt(kPirOutputPin), NoMoreMotion, FALLING);
}

void setup() {
  Serial.begin(9600);
  Serial.println("START");
  pinMode(kBuiltinLed, OUTPUT);
  digitalWrite(kBuiltinLed, LOW);

  pinMode(kPirOutputPin, INPUT);
  gMotion = false;
  gChange = false;
  attachInterrupt(digitalPinToInterrupt(kPirOutputPin), MovementDetected, RISING);
}

void loop() {
  if (gChange) {
    if (gMotion) {
      Serial.println("LOOP MOTION");
    }
    else {
      Serial.println("LOOP STEADY");
    }
    gChange = false;
  }
  
  if (gMotion) {
    digitalWrite(kBuiltinLed, HIGH);
  } else {
    digitalWrite(kBuiltinLed, LOW);
  }
}
