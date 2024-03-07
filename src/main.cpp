#include "cobra_algos.hpp"
#include "compiler.hpp"
#include "reader.hpp"
#include "preprocessor.hpp"

#include <iostream>
#include <vector>
#include <bitset>

int main() {
  Reader reader{std::cin};
  std::string line{};
  std::vector<std::vector<std::string>> token_lines{};
  while (getline(reader, line)) {
    token_lines.push_back(line2tokens(std::move(line)));
  }

  preprocess(token_lines);
  auto &preprocessed = token_lines;

  Compiler compiler{};

  auto &out_stream = std::cout;
  auto &error_stream = std::cerr;

  try {
    for (auto current_line_it = preprocessed.begin(); current_line_it != preprocessed.end(); ++current_line_it) {
      if (is_empty_line(*current_line_it)) continue;

      const char* const delim = " ";
      std::stringstream imploded;
      std::copy(current_line_it->begin(), current_line_it->end(),
                std::ostream_iterator<std::string>(imploded, delim));

      std::string str;
      imploded >> str;

      out_stream << std::bitset<32>(compiler.compile_str(str)) << "\n";
    }
  } catch (const Errors::Error &exc) {
    error_stream << "ERROR at line " << std::to_string(reader.get_current_line_number()) + " :\n"
                 << "  " << exc.what();
  }
}
