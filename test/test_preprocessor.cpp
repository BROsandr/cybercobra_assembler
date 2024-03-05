#include "preprocessor.hpp"

#define CATCH_CONFIG_MAIN
#include "catch2/catch_test_macros.hpp"

TEST_CASE("Preprocessor line2tokens", "[LINE2TOKENS]") {

  SECTION("hello world") {
    const std::vector<std::string> &tokens{line2tokens("hello world")};

    REQUIRE(tokens.size() == 2);
    REQUIRE(tokens.at(0) == "hello");
    REQUIRE(tokens.at(1) == "world");
  }

  SECTION("hello world \n ") {
    const std::vector<std::string> &tokens{line2tokens("hello world \n ")};

    REQUIRE(tokens.size() == 2);
    REQUIRE(tokens.at(0) == "hello");
    REQUIRE(tokens.at(1) == "world");
  }
}
