#include "preprocessor.hpp"

#include "reader.hpp"

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

TEST_CASE("Preprocessor handle_labels", "[HANDLE_LABELS]") {

  SECTION(
      R"""(test1:
        add x1, x2, x3
      )"""
  ) {
    std::stringstream ss{R"""(test1:
        add x1, x2, x3
      )"""};
    Reader reader{ss};
    std::string line{};
    std::vector<std::vector<std::string>> token_lines{};
    while (getline(reader, line)) {
      token_lines.push_back(line2tokens(std::move(line)));
    }

    handle_labels(token_lines);

    write_lines(token_lines);
  }

  SECTION(
      R"""(test1:

        add x1, x2, x3
        j  test1
      )"""
  ) {
    std::stringstream ss{R"""(test1:

        add x1, x2, x3
        j  test1
      )"""};
    Reader reader{ss};
    std::string line{};
    std::vector<std::vector<std::string>> token_lines{};
    while (getline(reader, line)) {
      token_lines.push_back(line2tokens(std::move(line)));
    }

    handle_labels(token_lines);

    write_lines(token_lines);
  }
}

TEST_CASE("Preprocessor calculate_next_instr_addr", "[CALCULATE_NEXT_INSTR_ADDR]") {
  SECTION("1, 2, instr") {
    std::vector<Line> lines{Line{"test1:"}, Line{"test2:"}, Line{"instr"}};
    std::map<Line_addr, std::string_view> labels{
        {lines.begin()  , "test1"},
        {lines.begin()+1, "test2"}
    };
    REQUIRE(calculate_next_instr_addr(labels.begin(),
        labels.end(), lines.end()) == lines.end() - 1);
  }
}
