#pragma once

#include <string>
#include <istream>

class Reader {
  public:
    friend Reader &getline(Reader &input, std::string &str);

    constexpr std::size_t get_current_line_number() const { return m_current_line_number; }

    constexpr Reader(std::istream &is) : m_is{is} {}
    Reader(const Reader& ) = delete;
    Reader(      Reader&&) = delete;

    Reader& operator=(const Reader& ) = delete;
    Reader& operator=(      Reader&&) = delete;
    Reader()  = delete;
    ~Reader() = default;

    constexpr operator bool() const { return static_cast<bool>(m_is); }

  private:
    std::istream &m_is;
    std::size_t m_current_line_number{static_cast<size_t>(-1)};
};

inline Reader &getline(Reader &input, std::string &str) {
  getline(input.m_is, str);
  ++input.m_current_line_number;
  return input;
}
