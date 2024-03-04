#pragma once

#include <string>
#include <iostream>
#include <optional>

class Reader {
  public:
    friend Reader &get_line(Reader &input, std::string &str) {
      std::getline(std::cin, str);
      if (input.m_current_line_number) {
        ++*input.m_current_line_number;
      } else {
        *input.m_current_line_number = 0;
      }
      return input;
    }

    constexpr std::size_t get_current_line_number() const {
      return *m_current_line_number;
    }

    Reader(const Reader& ) = delete;
    Reader(      Reader&&) = delete;

    Reader& operator=(const Reader& ) = delete;
    Reader& operator=(      Reader&&) = delete;

    static Reader& get_instance() {
      static Reader instance{};
      return instance;
    }

  private:
    Reader()  = default;
    ~Reader() = default;

    std::optional<std::size_t> m_current_line_number{};
};
