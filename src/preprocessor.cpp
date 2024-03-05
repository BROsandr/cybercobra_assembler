#include "preprocessor.hpp"

#include "reader.hpp"

#include <iostream>
#include <algorithm>


int main() {
  Reader reader{std::cin};
  std::string line{};
  std::vector<std::vector<std::string>> token_lines{};
  while (getline(reader, line)) {
    token_lines.push_back(line2tokens(std::move(line)));
  }

  handle_labels(token_lines);

  write_lines(token_lines);
}
