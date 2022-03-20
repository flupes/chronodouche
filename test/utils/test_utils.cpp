#include "unity.h"

#include "utils.h"

void test_rotate_left(void) {
  uint8_t start = 0b10011100;
  uint8_t data = RotateLeft(start, 1);
  TEST_ASSERT_EQUAL_UINT8(0b00111001, data);
  data = RotateLeft(data, 1);
  TEST_ASSERT_EQUAL_UINT8(0b01110010, data);
  data = RotateLeft(data, 6);
  TEST_ASSERT_EQUAL_UINT8(start, data);
}

void test_rotate_right(void) {
  uint8_t start = 0b10011100;
  uint8_t data = RotateRight(start, 3);
  TEST_ASSERT_EQUAL_UINT8(0b10010011, data);
  data = RotateRight(data, 1);
  TEST_ASSERT_EQUAL_UINT8(0b11001001, data);
  data = RotateRight(data, 4);
  TEST_ASSERT_EQUAL_UINT8(start, data);
}

#if defined(ARDUINO)
#include <Arduino.h>
void setup() {
#else
int main(int argc, char **argv) {
#endif
  UNITY_BEGIN();
  RUN_TEST(test_rotate_left);
  RUN_TEST(test_rotate_right);
  UNITY_END();
}

#if defined(ARDUINO)
void loop() {}
#endif