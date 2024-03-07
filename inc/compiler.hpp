#pragma once

#include "decoder.hpp"
#include "encoder.hpp"

class Compiler {
  public:
    Uxlen compile_str(const std::string &line) {

      Decoder::Instruction_info instr_info{decoder.decode(line)};
      return encoder.encode(instr_info);
    }

  private:
    Decoder decoder{};
    Encoder encoder{};
};
