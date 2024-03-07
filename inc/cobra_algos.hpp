#pragma once

#include <cstddef>
#include <string>
#include <algorithm>

template <typename T>
constexpr T make_mask(std::size_t len, std::size_t pos = 0) {
  return ((static_cast<T>(1) << len)-1) << pos;
}

constexpr bool is_empty_str(const std::string &str) {
  return std::all_of(str.begin(), str.end(),
      [](unsigned char c){ return std::isspace(c); });
}

template <typename Cont> requires (std::is_convertible_v<typename Cont::value_type, std::string>)
constexpr bool is_empty_line(const Cont &line) {
  return std::all_of(cbegin(line), cend(line), is_empty_str);
}
