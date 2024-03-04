#pragma once

#include "cobra.hpp"
#include "decoder.hpp"

class Encoder {
  public:
    Uxlen encode(Decoder::Instruction_info instr_info);
};
