#include "assembler.hpp"

#include <iostream>
#include <string>
#include <sstream>

#define CATCH_CONFIG_MAIN
#include "catch2/catch_test_macros.hpp"

TEST_CASE("Assembler assemble", "[ASSEMBLE]") {

  SECTION("empty line") {
    std::stringstream in{
        R"""(add x1, x2, x3
        add x1, x2, x3)"""
    };

    std::stringstream out{};

    Assembler assembler{};

    for (std::string str; getline(in, str);) out << assembler.assemble_str(str) << "\n";

    Uxlen line{};
    out >> line;
    REQUIRE(line == 0b00010000000010000110000000000001);
    line = {};
    out >> line;
    REQUIRE(line == 0b00010000000010000110000000000001);
    line = {};
    out >> line;
    REQUIRE(!out);
  }

  SECTION("blt x0, x1, 10") {

    std::string str{"blt x0, x1, 10"};

    Assembler assembler{};

    assembler.assemble_str(str);
    REQUIRE(assembler.assemble_str(str) == 0b01001110000000000010000101000000);
  }
}
