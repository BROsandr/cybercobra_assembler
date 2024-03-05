#include "decoder.hpp"

#include "exception.hpp"

#include <cassert>

#include <cstddef>
#include <iostream>
#include <sstream>

namespace {
  constexpr Decoder::Instruction_type concrete2type(Decoder::Concrete_instruction instruction) {
    using enum Decoder::Concrete_instruction;
    using enum Decoder::Instruction_type;
    switch (instruction) {

      case instr_add      :
      case instr_sub      :
      case instr_sll      :
      case instr_slts     :
      case instr_sltu     :
      case instr_xor      :
      case instr_srl      :
      case instr_sra      :
      case instr_or       :
      case instr_and      : return type_computational;

      case instr_blt :
      case instr_bltu:
      case instr_bge :
      case instr_bgeu:
      case instr_beq :
      case instr_bne : return type_branch;

      case instr_li  : return type_li;
      case instr_in  : return type_in;
      case instr_jump: return type_jump;

      default:
        assert(0 && "instr_number_ is not valid instruction");
        return type_computational;
    }

    assert(0 && "Unknown instruction");
  }

  void skip_sep(std::istream &line, char sep) {
    while (line.get() != sep) {
      throw Errors::Syntax_error("Unexpected eof, expected sep == " + std::to_string(sep));
    }
  }

  constexpr void extract_reg(const std::string &reg_name, auto &reg, std::istream &line) {
    char x;
    line >> x;
    line >> reg;

    if (!line || (x != 'x')) {
      throw Errors::Syntax_error("stringstream failed to extract a token. Expected " + reg_name);
    }
  }

  constexpr void extract_imm(const std::string &imm_name, auto &imm, std::istream &line) {
    line >> imm;

    if (!line) {
      throw Errors::Syntax_error("stringstream failed to extract a token. Expected " + imm_name);
    }
  }

  constexpr void decode_computational(Decoder::Instruction_info &info,
      std::istream &line, char sep = ',') {
    extract_reg("rd", info.rd, line);

    skip_sep(line, sep);
    extract_reg("rs1", info.rs1, line);

    skip_sep(line, sep);
    extract_reg("rs2", info.rs2, line);
  }

  constexpr void decode_li(Decoder::Instruction_info &info,
      std::istream &line, char sep = ',') {
    extract_reg("rd", info.rd, line);

    skip_sep(line, sep);
    extract_imm("imm", info.imm, line);
  }

  constexpr void decode_jump(Decoder::Instruction_info &info, std::istream &line) {
    line >> info.imm;

    if (!line) {
      throw Errors::Syntax_error("stringstream failed to extract a token. Expected imm");
    }
  }

  constexpr void decode_in(Decoder::Instruction_info &info, std::istream &line) {
    extract_reg("rd", info.rd, line);
  }

  constexpr void decode_branch(Decoder::Instruction_info &info, std::istream &line, char sep = ',') {
    extract_reg("rs1", info.rs1, line);

    skip_sep(line, sep);

    extract_reg("rs2", info.rs2, line);

    skip_sep(line, sep);

    extract_imm("imm", info.imm, line);
  }

  constexpr void decode_instruction_type(Decoder::Instruction_info &info, std::istream &line) {
    info.type = concrete2type(info.instruction);
    using enum Decoder::Instruction_type;
    switch (info.type) {
      case type_computational: decode_computational(info, line); break;
      case type_li           : decode_li           (info, line); break;
      case type_jump         : decode_jump         (info, line); break;
      case type_in           : decode_in           (info, line); break;
      case type_branch       : decode_branch       (info, line); break;
      default   : assert(0 && "Unexpected info.type");
    }
  }

  constexpr void check_ranges(const Decoder::Instruction_info &info) {
    auto min_max_check = [](std::size_t value, std::size_t min, std::size_t max, const std::string &name) {
      if ((value < min) || (value > max)) {
        throw Errors::Range_error(
            name ,
            "!(" + std::to_string(value) + " inside [" + std::to_string(min) + ":" + std::to_string(max) + "])");
      }
    };

    min_max_check(info.rd , 0, 31, "rd" );
    min_max_check(info.rs1, 0, 31, "rs1");
    min_max_check(info.rs2, 0, 31, "rs2");
    switch (info.type) {
      case Decoder::Instruction_type::type_jump  :
      case Decoder::Instruction_type::type_branch:
        min_max_check(info.imm, 0, 0b11111111, "imm");
        break;

      case Decoder::Instruction_type::type_li    :
        min_max_check(info.imm, 0, 0b11111111111111111111111, "imm");
        break;

      default: ;
    }
  }

  void check_extraneous_input(std::istream &&line) {
    char c{};
    while (line.get(c)) {
      if (!std::isspace(c)) {
        throw Errors::Syntax_error{"Etraneous_input other than \\n"};
      }
    }
  }
}

const std::map<std::string, Decoder::Concrete_instruction> Decoder::str_instr_mapping{
  {"add"      , Decoder::Concrete_instruction::instr_add      },
  {"sub"      , Decoder::Concrete_instruction::instr_sub      },
  {"xor"      , Decoder::Concrete_instruction::instr_xor      },
  {"or"       , Decoder::Concrete_instruction::instr_or       },
  {"and"      , Decoder::Concrete_instruction::instr_and      },
  {"sra"      , Decoder::Concrete_instruction::instr_sra      },
  {"sll"      , Decoder::Concrete_instruction::instr_sll      },
  {"srl"      , Decoder::Concrete_instruction::instr_srl      },
  {"blt"      , Decoder::Concrete_instruction::instr_blt      },
  {"bltu"     , Decoder::Concrete_instruction::instr_bltu     },
  {"bge"      , Decoder::Concrete_instruction::instr_bge      },
  {"bgeu"     , Decoder::Concrete_instruction::instr_bgeu     },
  {"beq"      , Decoder::Concrete_instruction::instr_beq      },
  {"bne"      , Decoder::Concrete_instruction::instr_bne      },
  {"slts"     , Decoder::Concrete_instruction::instr_slts     },
  {"sltu"     , Decoder::Concrete_instruction::instr_sltu     },
  {"li"       , Decoder::Concrete_instruction::instr_li       },
  {"in"       , Decoder::Concrete_instruction::instr_in       },
  {"j"        , Decoder::Concrete_instruction::instr_jump     },
};

Decoder::Concrete_instruction Decoder::decode_concrete_instruction(std::istream &line) const {

  std::string token{};
  line >> token;

  if (!line) {
    throw Errors::Syntax_error("stringstream failed to extract a token. Expected instruction");
  }

  try {
    return str_instr_mapping.at(token);
  } catch (const std::out_of_range &exception) {
    throw Errors::Illegal_instruction(token, exception.what());
  }
}

Decoder::Instruction_info Decoder::decode(const std::string &line) const {
  Instruction_info info{};
  std::stringstream ss{line};

  info.instruction = decode_concrete_instruction(ss);
  decode_instruction_type(info, ss);

  check_ranges(info);

  check_extraneous_input(std::move(ss));

  return info;
}

#ifdef UNIT_TEST
#define CATCH_CONFIG_MAIN
#include "catch2/catch_test_macros.hpp"

TEST_CASE("Decoder decode computational", "[DECODE_COMPUTATIONAL]") {

  SECTION("x1, x2, x3") {
    const std::string args{"x1, x2, x3"};
    Decoder::Instruction_info info{};
    std::stringstream         ss  {args};

    decode_computational(info, ss);
    REQUIRE(info.rd  == 1);
    REQUIRE(info.rs1 == 2);
    REQUIRE(info.rs2 == 3);
  }

  SECTION("x1,    x2,x3") {
    const std::string args{"x1,    x2,x3"};
    Decoder::Instruction_info info{};
    std::stringstream         ss  {args};

    decode_computational(info, ss);
    REQUIRE(info.rd  == 1);
    REQUIRE(info.rs1 == 2);
    REQUIRE(info.rs2 == 3);
  }

  SECTION("x1   x2,x3") {
    const std::string args{"x1   x2,x3"};
    Decoder::Instruction_info info{};
    std::stringstream         ss  {args};

    REQUIRE_THROWS_AS(decode_computational(info, ss), Errors::Syntax_error);
  }

  SECTION("x1, x2") {
    const std::string args{"x1, x2"};
    Decoder::Instruction_info info{};
    std::stringstream         ss  {args};

    REQUIRE_THROWS_AS(decode_computational(info, ss), Errors::Syntax_error);
  }
}

TEST_CASE("Decoder check_ranges", "[CHECK_RANGES]") {
  SECTION("rd == 34") {
    const Decoder::Instruction_info info{.rd = 34};

    REQUIRE_THROWS_AS(check_ranges(info), Errors::Range_error);
  }

  SECTION("rd == 0") {
    const Decoder::Instruction_info info{.rd = 0};

    REQUIRE_NOTHROW(check_ranges(info));
  }
}

TEST_CASE("Decoder check_extraneous_input", "[CHECK_EXTRANEOUS_INPUT]") {
  SECTION("   ") {
    REQUIRE_NOTHROW(check_extraneous_input(std::stringstream{"   "}));
  }

  SECTION("") {
    REQUIRE_NOTHROW(check_extraneous_input(std::stringstream{""}));
  }

  SECTION("  ab  ") {
    REQUIRE_NOTHROW(check_extraneous_input(std::stringstream{""}));
  }
}

TEST_CASE("Decoder decode", "[DECODE]") {
  SECTION("add x1, x2, x3") {
    const std::string line{"add x1, x2, x3"};
    Decoder decoder{};
    Decoder::Instruction_info info{decoder.decode(line)};
    REQUIRE(info.instruction == Decoder::Concrete_instruction::instr_add);
    REQUIRE(info.type        == Decoder::Instruction_type::type_computational);
    REQUIRE(info.rd          == 1);
    REQUIRE(info.rs1         == 2);
    REQUIRE(info.rs2         == 3);
  }

  SECTION("addi x1, x2, x3") {
    const std::string line{"addi 1, 2, 3"};
    Decoder decoder{};
    REQUIRE_THROWS_AS(decoder.decode(line), Errors::Illegal_instruction);
  }

  SECTION("j x3, x1, x2, x3") {
    const std::string line{"j 3, 1, 2, 3"};
    Decoder decoder{};
    REQUIRE_THROWS_AS(decoder.decode(line), Errors::Syntax_error);
  }

  SECTION("j 20") {
    const std::string line{"j 20"};
    Decoder decoder{};
    Decoder::Instruction_info info{decoder.decode(line)};
    REQUIRE(info.instruction == Decoder::Concrete_instruction::instr_jump);
    REQUIRE(info.type        == Decoder::Instruction_type::type_jump);
    REQUIRE(info.imm         == 20);
  }

  SECTION("bne x20, x0, 19") {
    const std::string line{"bne x20, x0, 19"};
    Decoder decoder{};
    Decoder::Instruction_info info{decoder.decode(line)};
    REQUIRE(info.instruction == Decoder::Concrete_instruction::instr_bne);
    REQUIRE(info.type        == Decoder::Instruction_type::type_branch);
    REQUIRE(info.rs1         == 20);
    REQUIRE(info.rs2         ==  0);
    REQUIRE(info.imm         == 19);
  }

  SECTION("in x20") {
    const std::string line{"in x20"};
    Decoder decoder{};
    Decoder::Instruction_info info{decoder.decode(line)};
    REQUIRE(info.instruction == Decoder::Concrete_instruction::instr_in);
    REQUIRE(info.type        == Decoder::Instruction_type::type_in);
    REQUIRE(info.rd          == 20);
  }

  SECTION("li x3, 300") {
    const std::string line{"li x3, 300"};
    Decoder decoder{};
    Decoder::Instruction_info info{decoder.decode(line)};
    REQUIRE(info.instruction == Decoder::Concrete_instruction::instr_li);
    REQUIRE(info.type        == Decoder::Instruction_type::type_li);
    REQUIRE(info.rd          == 3);
    REQUIRE(info.imm         == 300);
  }

  SECTION("li x300, 300") {
    const std::string line{"li x300, 300"};
    Decoder decoder{};
    REQUIRE_THROWS_AS(decoder.decode(line), Errors::Range_error);
  }
}
#endif
