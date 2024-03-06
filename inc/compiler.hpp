#pragma once

#include "decoder.hpp"
#include "encoder.hpp"
#include "exception.hpp"
#include "reader.hpp"

#include <bitset>

template <typename I, typename W, typename E>
void compile(I &&in_stream, W &&out_stream, E &&error_stream) {
  Decoder decoder{};
  Encoder encoder{};

  std::string line{};

  Reader reader{std::forward<I>(in_stream)};

  try {
    while (getline(reader, line)) {
      if (line.empty()) continue;
      Decoder::Instruction_info instr_info{decoder.decode(std::move(line))};
      Uxlen instruction{encoder.encode(instr_info)};
      out_stream << std::bitset<32>(instruction) << "\n";
    }
  } catch (const Errors::Error &exc) {
    error_stream << "ERROR at line " << std::to_string(reader.get_current_line_number()) + " :\n"
                 << "  " << exc.what();
  }
}
