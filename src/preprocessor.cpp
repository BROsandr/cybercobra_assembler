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

  auto &writer = std::cout;

  for (auto &line_el : token_lines) {

    std::copy(line_el.begin(), line_el.end(), std::ostream_iterator<std::string>{writer, " "});
    std::cout << "\n";
  }
}
