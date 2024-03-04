#pragma once

#include <stdexcept>

namespace Errors {
  struct Error : public std::runtime_error {
    using std::runtime_error::runtime_error;
  };

  struct Illegal_instruction : public Error {
    const std::string m_instruction;

    Illegal_instruction(const std::string &instruction, const std::string &message = "")
        : Error(instruction + " : " + message),  m_instruction{instruction} {}
  };

  struct Syntax_error : public Error {

    Syntax_error(const std::string &message = "")
        : Error( message) {}
  };

  struct Range_error : public Error {
    Range_error(const std::string &who, const std::string &message)
        : Error(who + " : " + message) {}
  };

}
