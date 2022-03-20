#pragma once

#include <stdint.h>

template <typename T> T RotateLeft(T data, size_t offset) {
  return (data << offset) | (data >> (8*sizeof(T) - offset));
}

template <typename T> T RotateRight(T data, size_t offset) {
  return (data >> offset) | (data << (8*sizeof(T) - offset));
}
