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

    handle_labels(token_lines);

    REQUIRE(token_lines == std::vector<std::vector<std::string>>{
        {""},
        {"add", "x1", "x2", "x3"}
    });
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
        {""},
        {""},
        {"add", "x1", "x2", "x3"},
        {"j", "-1"},
    });
  }

  SECTION(
      R"""(
      test1:
      test2:
        li  x1, 3
        add x1, x2, x3
        j   20
        j   test1
        j   test2

      test3:
        bge x3, x31, test1
      )"""
  ) {
    std::vector<std::vector<std::string>> token_lines{
        {"test1:"},
        {"test2:"},
        {"li", "x1", "3"},
        {"add", "x1", "x2", "x3"},
        {"j", "20"},
        {"j", "test1"},
        {"j", "test2"},
        {""},
        {"test3:"},
        {"bge", "x3", "x31", "test1"},
    };

    handle_labels(token_lines);

    REQUIRE(token_lines == std::vector<std::vector<std::string>>{
        {""},
        {""},
        {"li", "x1", "3"},
        {"add", "x1", "x2", "x3"},
        {"j", "20"},
        {"j", "-3"},
        {"j", "-4"},
        {""},
        {""},
        {"bge", "x3", "x31", "-5"},
    });
  }

  SECTION(
      R"""(
      j test1
      test1:
      j test1
      )"""
  ) {
    std::vector<std::vector<std::string>> token_lines{
        {"j", "test1"},
        {"test1:"},
        {"j", "test1"},
    };

    handle_labels(token_lines);

    REQUIRE(token_lines == std::vector<std::vector<std::string>>{
        {"j", "1"},
        {""},
        {"j", "0"},
    });
  }

  SECTION(
      R"""(
      j test1
      test1:
      add x1, x2, x3
      j test1
      )"""
  ) {
    std::vector<std::vector<std::string>> token_lines{
        {"j", "test1"},
        {"test1:"},
        {"add", "x1", "x2", "x3"},
        {"j", "test1"},
    };

    handle_labels(token_lines);

    REQUIRE(token_lines == std::vector<std::vector<std::string>>{
        {"j", "1"},
        {""},
        {"add", "x1", "x2", "x3"},
        {"j", "-1"},
    });
  }

  SECTION(
      R"""(
      j test1
      test1:
      add x1, x2, x3
      j test1
      )"""
  ) {
    std::vector<std::vector<std::string>> token_lines{
      {"blt",  "x1", "x2", "blt_forth"},
      {"blt_back:"},
      {""},
      {"bltu", "x1", "x2", "bltu_forth"},
      {"bltu_back:"},
      {""},
      {"blt_forth:"},
      {"j", "blt_back"},
      {"bltu_forth:"},
      {"j", "bltu_back"},
    };

    handle_labels(token_lines);

    REQUIRE(token_lines == std::vector<std::vector<std::string>>{
      {"blt",  "x1", "x2", "2"},
      {""},
      {""},
      {"bltu", "x1", "x2", "2"},
      {""},
      {""},
      {""},
      {"j", "-1"},
      {""},
      {"j", "-1"},
    });
  }
}

TEST_CASE("Preprocessor remove_labels", "[REMOVE_LABELS]") {
  SECTION("1, 2, instr") {
    std::vector<Line> lines{Line{"test1:"}, Line{"test2:"}, Line{"instr"}};
    Labels labels{
        {"test1", lines.begin()},
        {"test2", lines.begin()+1}
    };
    REQUIRE(remove_labels(lines,labels) == std::vector{lines.end()-1});
  }

  SECTION("2") {
    std::vector<std::vector<std::string>> token_lines{
        {"test1:"},
        {"test2:"},
        {"li", "x1", "3"},
        {"add", "x1", "x2", "x3"},
        {"j", "20"},
        {"j", "test1"},
        {"j", "test2"},
        {""},
        {"test3:"},
        {"bge", "x3", "x31", "test1"},
    };

    Labels labels{
        {"test1", token_lines.begin()},
        {"test2", token_lines.begin()+1},
        {"test3", token_lines.end()-2},
    };

    REQUIRE(remove_labels(token_lines, labels) == std::vector<Line_addr>{
        token_lines.begin()+2,
        token_lines.begin()+3,
        token_lines.begin()+4,
        token_lines.begin()+5,
        token_lines.begin()+6,
        token_lines.begin()+7,
        token_lines.end()-1,
    });
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

  SECTION("2") {
    std::vector<std::vector<std::string>> token_lines{
        {"test1:"},
        {"test2:"},
        {"li", "x1", "3"},
        {"add", "x1", "x2", "x3"},
        {"j", "20"},
        {"j", "test1"},
        {"j", "test2"},
        {""},
        {"test3:"},
        {"bge", "x3", "x31", "test1"},
    };

    std::vector<Line_addr> lines{
        token_lines.begin()+2,
        token_lines.begin()+3,
        token_lines.begin()+4,
        token_lines.begin()+5,
        token_lines.begin()+6,
        token_lines.begin()+7,
        token_lines.end()-1,
    };

    REQUIRE(remove_empty_lines(lines) == std::vector<Line_addr>{
        token_lines.begin()+2,
        token_lines.begin()+3,
        token_lines.begin()+4,
        token_lines.begin()+5,
        token_lines.begin()+6,
        token_lines.end()-1,
    });
  }
}

TEST_CASE("Preprocessor calculate_next_instr_addr", "[CALCULATE_NEXT_INSTR_ADDR]") {
  SECTION("1, 2, instr") {
    std::vector<Line> lines{Line{"test1:"}, Line{"test2:"}, Line{"instr"}};
    Labels labels{
        {"test1", lines.begin()  },
        {"test2", lines.begin()+1}
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
    Labels labels{
        {"test1", lines.begin()},
    };
    REQUIRE(calculate_next_instr_addr(labels.begin(),
        labels.end(), lines.end()) == lines.end() - 2);
  }

  SECTION("calculate_next_instr_addr 3") {
    std::vector<Line> lines{
      {"bltu_back:"},
      {""},
      {"blt_forth:"},
      {"j", "blt_back"},
    };
    Labels labels{
        std::pair<std::string_view, Line_addr>{"bltu_back", lines.begin()},
        std::pair<std::string_view, Line_addr>{"blt_forth", lines.begin()+2},
    };
    REQUIRE(calculate_next_instr_addr(labels.begin(),
        labels.end(), lines.end()) == lines.end() - 1);
    REQUIRE(calculate_next_instr_addr(next(labels.begin()),
        labels.end(), lines.end()) == lines.end() - 1);
  }
}
