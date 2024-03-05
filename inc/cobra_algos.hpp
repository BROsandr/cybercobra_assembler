#pragma once

#include <cstddef>

template <typename T>
constexpr T make_mask(std::size_t len, std::size_t pos = 0) {
  return ((static_cast<T>(1) << len)-1) << pos;
}
