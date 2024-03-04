#include "decoder.hpp"
#include "encoder.hpp"

#include <iostream>
#include <bitset>

int main() {
  Decoder decoder{};
  Encoder encoder{};

  std::string line{};

  auto &reader = std::cin;
  auto &writer = std::cout;

  while (std::getline(reader, line)) {
    Decoder::Instruction_info instr_info{decoder.decode(line)};
    Uxlen instruction{encoder.encode(instr_info)};
    writer << std::bitset<32>(instruction) << "\n";
  }

  return 0;
}
