#include "compiler.hpp"

#include <iostream>
#include <string>
#include <sstream>

#define CATCH_CONFIG_MAIN
#include "catch2/catch_test_macros.hpp"

TEST_CASE("Compiler compile", "[COMPILE]") {

  SECTION("empty line") {
    std::stringstream in{
        R"""(add x1, x2, x3
        add x1, x2, x3)"""
    };

    std::stringstream out{};

    Compiler compiler{};

    for (std::string str; getline(in, str);) out << compiler.compile_str(str) << "\n";

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

    Compiler compiler{};

    compiler.compile_str(str);
    REQUIRE(compiler.compile_str(str) == 0b01001110000000000010000101000000);
  }
}
