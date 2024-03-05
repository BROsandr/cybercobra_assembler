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

TEST_CASE("Preprocessor calculate_abs_addr", "[CALCULATE_ABS_ADDR]") {
  SECTION("1, 2, 4") {
    std::map<std::size_t, std::string> labels{{1, ""}, {2, ""}, {4, ""}};
    std::vector<std::vector<std::string>> lines{};
    for ([[maybe_unused]]auto &el: labels) {
      lines.push_back({{" "}});
    }
    REQUIRE(calculate_abs_addr(labels.begin(), labels.end()) == 3);
  }

  SECTION("0, 2, 3, 4, 18") {
    std::map<std::size_t, std::string> labels{{0, ""}, {2, ""}, {3, ""}, {4, ""}, {18, ""}};
    std::vector<std::vector<std::string>> lines{};
    for ([[maybe_unused]]auto &el: labels) {
      lines.push_back({{" "}});
    }
    REQUIRE(calculate_abs_addr(next(labels.begin()), labels.end()) == 5);
  }
}
