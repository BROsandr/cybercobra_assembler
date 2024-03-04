#include "decoder.hpp"

#include "exception.hpp"

#include <cassert>
#include <climits>

#include <cstddef>
#include <initializer_list>
#include <iostream>
#include <sstream>

namespace {
  class Bit_range {
    public:
      constexpr Bit_range(std::size_t msb, std::size_t lsb)
          : m_msb{msb}, m_lsb{lsb}, m_width{msb - lsb + 1} {
        assert(msb >= lsb);
      }
      explicit constexpr Bit_range(std::size_t pos)
          : Bit_range(pos, pos) {}

      constexpr std::size_t get_msb()   const {return m_msb;}
      constexpr std::size_t get_lsb()   const {return m_lsb;}
      constexpr std::size_t get_width() const {return m_width;}

      constexpr bool is_overlapping(const Bit_range& rhs) const {
        return get_lsb() <= rhs.get_msb() && rhs.get_lsb() <= get_msb();
      }

      friend std::ostream& operator<<(std::ostream& os, const Bit_range& range) {
          os << "{" << range.get_msb() << "," << range.get_lsb() << "}";
          return os;
      }

    private:
      const std::size_t m_msb;
      const std::size_t m_lsb;
      const std::size_t m_width;
  };

  constexpr bool is_overlapping(const Bit_range& lhs, const Bit_range& rhs) {
    return lhs.get_lsb() <= rhs.get_msb() && rhs.get_lsb() <= lhs.get_msb();
  }

  template <typename T>
  constexpr T make_mask(std::size_t len, std::size_t pos = 0) {
    return ((static_cast<T>(1) << len)-1) << pos;
  }

  template<typename T>
  concept Right_shiftable = requires(T a) {
    a >> 1;
  };

  template<typename T>
  concept Andable = requires(T a) {
    a & static_cast<T>(1);
  };

  template <typename T>
  constexpr T sign_extend(T data, std::size_t sign_pos) {
    assert(sign_pos < (sizeof(data) * 8));

    T m{T{1} << (sign_pos-1)};
    return (data ^ m) - m;
  }

  template <typename T> requires Right_shiftable<T> && Andable<T>
  constexpr T extract_bits(T data, Bit_range range, bool sext = false) {
    assert(range.get_msb() < (sizeof(T) * CHAR_BIT));
    T unextended{(data >> range.get_lsb()) & static_cast<T>(make_mask<unsigned long long>(range.get_width()))};
    if (sext) return sign_extend(unextended, range.get_width() - 1);
    else      return unextended;
  }

  template <typename T> requires Right_shiftable<T> && Andable<T>
  constexpr T extract_bits(T data, std::size_t pos, bool sext = false) {
    return extract_bits(data, {pos, pos}, sext);
  }

  template <typename T>
  concept Left_shiftable = requires(T a) {
    a << 1;
  };

  template <typename T>
  concept Shiftable = Left_shiftable<T> && Right_shiftable<T>;

  template <typename T>
  concept Orable = requires(T a) {
    a | static_cast<T>(1);
  };

  template <typename T> requires Shiftable<T> && Orable<T>
  constexpr T extract_bits(T data, std::initializer_list<Bit_range> bit_ranges, bool sext = false) {
    T result{extract_bits(data, *(bit_ranges.begin()), sext)};
    for (const auto *it{bit_ranges.begin() + 1}; it != bit_ranges.end(); ++it) {
      result = (result << (*it).get_width()) | extract_bits(data, *it);
    }
    return result;
  }

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

      case instr_const    : return type_const;
      case instr_periphery: return type_periphery;
      case instr_jump     : return type_jump;

      default:
        assert("instr_number_ is not valid instruction");
        return type_none;
    }

    assert("Unknown instruction");
  }

  void skip_sep(std::istream &line, char sep) {
    while (line.get() != sep) {
      throw Errors::Syntax_error("Unexpected eof, expected sep == " + std::to_string(sep));
    }
  }

  constexpr void decode_computational(Decoder::Instruction_info &info,
      std::istream &line, char sep = ',') {
    line >> info.rd;

    if (!line) {
      throw Errors::Syntax_error("stringstream failed to extract a token. Expected rd");
    }

    skip_sep(line, sep);
    line >> info.rs1;

    if (!line) {
      throw Errors::Syntax_error("stringstream failed to extract a token. Expected rs1");
    }

    skip_sep(line, sep);
    line >> info.rs2;

    if (!line) {
      throw Errors::Syntax_error("stringstream failed to extract a token. Expected rs2");
    }
  }

  constexpr void decode_const(Decoder::Instruction_info &info,
      std::istream &line, char sep = ',') {
    line >> info.rd;

    if (!line) {
      throw Errors::Syntax_error("stringstream failed to extract a token. Expected rd");
    }

    skip_sep(line, sep);
    line >> info.imm;

    if (!line) {
      throw Errors::Syntax_error("stringstream failed to extract a token. Expected rs1");
    }
  }

  constexpr void decode_jump(Decoder::Instruction_info &info, std::istream &line, char sep = ',') {
    line >> info.imm;

    if (!line) {
      throw Errors::Syntax_error("stringstream failed to extract a token. Expected imm");
    }
  }

  constexpr void decode_periphery(Decoder::Instruction_info &info, std::istream &line, char sep = ',') {
    line >> info.rd;

    if (!line) {
      throw Errors::Syntax_error("stringstream failed to extract a token. Expected rd");
    }
  }

  constexpr void decode_branch(Decoder::Instruction_info &info, std::istream &line, char sep = ',') {
    line >> info.rs1;

    if (!line) {
      throw Errors::Syntax_error("stringstream failed to extract a token. Expected rs1");
    }

    skip_sep(line, sep);

    line >> info.rs2;

    if (!line) {
      throw Errors::Syntax_error("stringstream failed to extract a token. Expected rs2");
    }

    skip_sep(line, sep);

    line >> info.imm;

    if (!line) {
      throw Errors::Syntax_error("stringstream failed to extract a token. Expected imm");
    }
  }

  constexpr void decode_instruction_type(Decoder::Instruction_info &info, std::istream &line) {
    info.type = concrete2type(info.instruction);
    using enum Decoder::Instruction_type;
    switch (info.type) {
      case type_none         :                                   break;
      case type_computational: decode_computational(info, line); break;
      case type_const        : decode_const        (info, line); break;
      case type_jump         : decode_jump         (info, line); break;
      case type_periphery    : decode_periphery    (info, line); break;
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

      case Decoder::Instruction_type::type_const :
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

Decoder::Decoder() {
  using enum Concrete_instruction;

  str_instr_mapping["add"      ] = instr_add      ;
  str_instr_mapping["sub"      ] = instr_sub      ;
  str_instr_mapping["xor"      ] = instr_xor      ;
  str_instr_mapping["or"       ] = instr_or       ;
  str_instr_mapping["and"      ] = instr_and      ;
  str_instr_mapping["sra"      ] = instr_sra      ;
  str_instr_mapping["sll"      ] = instr_sll      ;
  str_instr_mapping["srl"      ] = instr_srl      ;
  str_instr_mapping["blt"      ] = instr_blt      ;
  str_instr_mapping["bltu"     ] = instr_bltu     ;
  str_instr_mapping["bge"      ] = instr_bge      ;
  str_instr_mapping["bgeu"     ] = instr_bgeu     ;
  str_instr_mapping["beq"      ] = instr_beq      ;
  str_instr_mapping["bne"      ] = instr_bne      ;
  str_instr_mapping["slts"     ] = instr_slts     ;
  str_instr_mapping["sltu"     ] = instr_sltu     ;
  str_instr_mapping["const"    ] = instr_const    ;
  str_instr_mapping["periphery"] = instr_periphery;
  str_instr_mapping["j"        ] = instr_jump     ;
}

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

  SECTION("1, 2, 3") {
    const std::string args{"1, 2, 3"};
    Decoder::Instruction_info info{};
    std::stringstream         ss  {args};

    decode_computational(info, ss);
    REQUIRE(info.rd  == 1);
    REQUIRE(info.rs1 == 2);
    REQUIRE(info.rs2 == 3);
  }

  SECTION("1,    2,3") {
    const std::string args{"1,    2,3"};
    Decoder::Instruction_info info{};
    std::stringstream         ss  {args};

    decode_computational(info, ss);
    REQUIRE(info.rd  == 1);
    REQUIRE(info.rs1 == 2);
    REQUIRE(info.rs2 == 3);
  }

  SECTION("1   2,3") {
    const std::string args{"1   2,3"};
    Decoder::Instruction_info info{};
    std::stringstream         ss  {args};

    REQUIRE_THROWS_AS(decode_computational(info, ss), Errors::Syntax_error);
  }

  SECTION("1, 2") {
    const std::string args{"1, 2"};
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
  SECTION("add 1, 2, 3") {
    const std::string line{"add 1, 2, 3"};
    Decoder decoder{};
    Decoder::Instruction_info info{decoder.decode(line)};
    REQUIRE(info.instruction == Decoder::Concrete_instruction::instr_add);
    REQUIRE(info.type        == Decoder::Instruction_type::type_computational);
    REQUIRE(info.rd          == 1);
    REQUIRE(info.rs1         == 2);
    REQUIRE(info.rs2         == 3);
  }

  SECTION("addi 1, 2, 3") {
    const std::string line{"addi 1, 2, 3"};
    Decoder decoder{};
    REQUIRE_THROWS_AS(decoder.decode(line), Errors::Illegal_instruction);
  }

  SECTION("j 3, 1, 2, 3") {
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

  SECTION("bne 20, 0, 19") {
    const std::string line{"bne 20, 0, 19"};
    Decoder decoder{};
    Decoder::Instruction_info info{decoder.decode(line)};
    REQUIRE(info.instruction == Decoder::Concrete_instruction::instr_bne);
    REQUIRE(info.type        == Decoder::Instruction_type::type_branch);
    REQUIRE(info.rs1         == 20);
    REQUIRE(info.rs2         ==  0);
    REQUIRE(info.imm         == 19);
  }

  SECTION("periphery 20") {
    const std::string line{"periphery 20"};
    Decoder decoder{};
    Decoder::Instruction_info info{decoder.decode(line)};
    REQUIRE(info.instruction == Decoder::Concrete_instruction::instr_periphery);
    REQUIRE(info.type        == Decoder::Instruction_type::type_periphery);
    REQUIRE(info.rd          == 20);
  }

  SECTION("const 3, 300") {
    const std::string line{"const 3, 300"};
    Decoder decoder{};
    Decoder::Instruction_info info{decoder.decode(line)};
    REQUIRE(info.instruction == Decoder::Concrete_instruction::instr_const);
    REQUIRE(info.type        == Decoder::Instruction_type::type_const);
    REQUIRE(info.rd          == 3);
    REQUIRE(info.imm         == 300);
  }

  SECTION("const 300, 300") {
    const std::string line{"const 300, 300"};
    Decoder decoder{};
    REQUIRE_THROWS_AS(decoder.decode(line), Errors::Range_error);
  }
}
#endif
