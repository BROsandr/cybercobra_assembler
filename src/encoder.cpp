#include "encoder.hpp"

#include "decoder.hpp"

#include <cassert>

namespace {
  constexpr Uxlen encode_li(Decoder::Instruction_info instr_info) {
    Uxlen instr{0};
    instr = instr_info.imm;
    instr = (instr << 5) | instr_info.rd;

    return instr;
  }

  enum Alu_op : unsigned int {
    alu_add  = 0b00000,
    alu_sub  = 0b01000,
    alu_xor  = 0b00100,
    alu_or   = 0b00110,
    alu_and  = 0b00111,
    alu_sra  = 0b01101,
    alu_srl  = 0b00101,
    alu_sll  = 0b00001,
    alu_lts  = 0b11100,
    alu_ltu  = 0b11110,
    alu_ges  = 0b11101,
    alu_geu  = 0b11111,
    alu_eq   = 0b11000,
    alu_ne   = 0b11001,
    alu_slts = 0b00010,
    alu_sltu = 0b00011,
  };

  constexpr Alu_op instr2alu_op(Decoder::Concrete_instruction instr) {
    using enum Decoder::Concrete_instruction;
    using enum Alu_op;
    switch (instr) {
      case instr_add : return alu_add;
      case instr_sub : return alu_sub;
      case instr_xor : return alu_xor;
      case instr_or  : return alu_or;
      case instr_and : return alu_and;
      case instr_sra : return alu_sra;
      case instr_srl : return alu_srl;
      case instr_sll : return alu_sll;
      case instr_blt : return alu_lts;
      case instr_bltu: return alu_ltu;
      case instr_bge : return alu_ges;
      case instr_bgeu: return alu_geu;
      case instr_beq : return alu_eq;
      case instr_bne : return alu_ne;
      case instr_slts: return alu_slts;
      case instr_sltu: return alu_sltu;

      default:
        assert(0 && "Unexpected alu_op");
        return alu_add;
    }
  }

  constexpr Uxlen encode_branch(Decoder::Instruction_info instr_info) {
    Uxlen instr{0};
    instr = 1;
    instr = (instr << 7) | instr2alu_op(instr_info.instruction);
    instr = (instr << 5) | instr_info.rs1;
    instr = (instr << 5) | instr_info.rs2;
    instr = (instr << 13);

    return instr;
  }

  constexpr Uxlen encode_computational(Decoder::Instruction_info instr_info) {
    Uxlen instr{0};
    instr = 1;
    instr = (instr << 5)  | instr2alu_op(instr_info.instruction);
    instr = (instr << 5)  | instr_info.rs1;
    instr = (instr << 5)  | instr_info.rs2;
    instr = (instr << 13) | instr_info.rd;

    return instr;
  }

  constexpr Uxlen encode_jump(Decoder::Instruction_info instr_info) {
    Uxlen instr{0};
    instr = 1;
    instr = (instr << 26) | instr_info.imm;
    instr = instr << 5;

    return instr;
  }

  constexpr Uxlen encode_periphery(Decoder::Instruction_info instr_info) {
    Uxlen instr{0};
    instr = 1;
    instr = (instr << 29) | instr_info.rd;

    return instr;
  }
}

Uxlen Encoder::encode(Decoder::Instruction_info instr_info) {
  switch (instr_info.type) {
    case Decoder::Instruction_type::type_li           : return encode_li           (instr_info);
    case Decoder::Instruction_type::type_branch       : return encode_branch       (instr_info);
    case Decoder::Instruction_type::type_computational: return encode_computational(instr_info);
    case Decoder::Instruction_type::type_jump         : return encode_jump         (instr_info);
    case Decoder::Instruction_type::type_periphery    : return encode_periphery    (instr_info);
    default:
      assert(0 && "Unexcpected type");
      return 0xb1bab0ba;
  }
}

#ifdef UNIT_TEST
#define CATCH_CONFIG_MAIN
#include "catch2/catch_test_macros.hpp"

TEST_CASE("Decoder encode computational", "[ENCODE_COMPUTATIONAL]") {

  Encoder encoder{};

  SECTION("add x1, x2, x3") {
    const Uxlen instr{0b00010000000010000110000000000001};
    Decoder::Instruction_info instr_info{};
    instr_info.rd          = 1;
    instr_info.instruction = Decoder::Concrete_instruction::instr_add;
    instr_info.type        = Decoder::Instruction_type::type_computational;
    instr_info.rs1         = 2;
    instr_info.rs2         = 3;

    REQUIRE(encoder.encode(instr_info) == instr);
  }
}

TEST_CASE("Decoder encode computational", "[ENCODE_JUMP]") {

  Encoder encoder{};

  SECTION("j   20") {
    const Uxlen instr{0b10000000000000000000001010000000};
    Decoder::Instruction_info instr_info{};
    instr_info.imm         = 20;
    instr_info.instruction = Decoder::Concrete_instruction::instr_jump;
    instr_info.type        = Decoder::Instruction_type::type_jump;

    REQUIRE(encoder.encode(instr_info) == instr);
  }
}
#endif
