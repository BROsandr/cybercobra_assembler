#include "decoder.hpp"
#include "encoder.hpp"
#include "exception.hpp"
#include "reader.hpp"

#include <iostream>
#include <bitset>

int main() {
  Decoder decoder{};
  Encoder encoder{};

  std::string line{};

  auto &reader = Reader::get_instance();
  auto &writer = std::cout;
  try {
    while (getline(reader, line)) {
      Decoder::Instruction_info instr_info{decoder.decode(line)};
      Uxlen instruction{encoder.encode(instr_info)};
      writer << std::bitset<32>(instruction) << "\n";
    }
  } catch (const Errors::Error &exc) {
    std::cerr << std::to_string(reader.get_current_line_number()) + " :\n"
              << "  " << exc.what();
  }

  return 0;
}
