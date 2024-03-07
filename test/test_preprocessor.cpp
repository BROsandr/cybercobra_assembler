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
    std::vector<std::vector<std::string>> token_lines{
        {"test1:"},
        {"add", "x1", "x2", "x3"},
    };

    auto token_lines_copy = token_lines;

    handle_labels(token_lines_copy);

    REQUIRE(token_lines_copy == token_lines);
  }

  SECTION(
      R"""(test1:

        add x1, x2, x3
        j  test1
      )"""
  ) {
    std::vector<std::vector<std::string>> token_lines{
        {"test1:"},
        {""},
        {"add", "x1", "x2", "x3"},
        {"j", "test1"},
    };

    handle_labels(token_lines);

    REQUIRE(token_lines == std::vector<std::vector<std::string>>{
        {"test1:"},
        {""},
        {"add", "x1", "x2", "x3"},
        {"j", "-1"},
    });
  }
}

TEST_CASE("Preprocessor remove_labels", "[REMOVE_LABELS]") {
  SECTION("1, 2, instr") {
    std::vector<Line> lines{Line{"test1:"}, Line{"test2:"}, Line{"instr"}};
    std::map<Line_addr, std::string_view> labels{
        {lines.begin()  , "test1"},
        {lines.begin()+1, "test2"}
    };
    REQUIRE(remove_labels(lines,labels) == std::vector{lines.end()-1});
  }
}

TEST_CASE("Preprocessor empty_lines", "[EMPTY_LINES]") {
  SECTION("1, , , instr") {
    std::vector<Line> file{Line{"test1:"}, Line{}, Line{}, Line{"instr"}};

    std::vector<Line_addr> lines{};
    for (auto it = file.begin(); it != file.end(); ++it) {
      lines.push_back(it);
    }

    REQUIRE(remove_empty_lines(lines) == std::vector{file.begin(), file.end()-1});
  }

  SECTION("1, "", "", instr") {
    std::vector<Line> file{Line{"test1:"}, Line{""}, Line{""}, Line{"instr"}};

    std::vector<Line_addr> lines{};
    for (auto it = file.begin(); it != file.end(); ++it) {
      lines.push_back(it);
    }

    REQUIRE(remove_empty_lines(lines) == std::vector{file.begin(), file.end()-1});
  }

  SECTION("1, ' \n', ' ', instr") {
    std::vector<Line> file{Line{"test1:"}, Line{" \n"}, Line{" "}, Line{"instr"}};

    std::vector<Line_addr> lines{};
    for (auto it = file.begin(); it != file.end(); ++it) {
      lines.push_back(it);
    }

    REQUIRE(remove_empty_lines(lines) == std::vector{file.begin(), file.end()-1});
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

  SECTION("calculate_next_instr_addr 2") {
    std::vector<Line> lines{
        Line{"test1:"},
        Line{""},
        Line{"add", "x1", "x2", "x3"},
        Line{"j", "test1"},
    };
    std::map<Line_addr, std::string_view> labels{
        {lines.begin()  , "test1"},
    };
    REQUIRE(calculate_next_instr_addr(labels.begin(),
        labels.end(), lines.end()) == lines.end() - 2);
  }
}
