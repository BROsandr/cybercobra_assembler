#pragma once

#include "exception.hpp"

#include <iterator>
#include <vector>
#include <string>
#include <sstream>
#include <map>
#include <algorithm>

inline std::vector<std::string> line2tokens(const std::string &line) {
  std::stringstream ss{line};
  std::vector<std::string> tokens{};
  while (true) {
    std::string token{};
    ss >> token;
    if (!ss) break;
    tokens.push_back(std::move(token));
  }
  return tokens;
}

inline std::size_t calculate_abs_addr(std::map<std::size_t, std::string>::const_iterator labels_it,
    std::map<std::size_t, std::string>::const_iterator end_it) {
  while ((next(labels_it) != end_it) && (next(labels_it)->first == (labels_it->first + 1))) {
    ++labels_it;
  }
  return labels_it->first + 1;
}

inline void handle_labels(std::vector<std::vector<std::string>> &token_lines) {
  std::map<std::size_t, std::string> labels;
  for (std::size_t i{0}; i < token_lines.size(); ++i) {
    const std::string &first_token{token_lines[i][0]};
    if (first_token.back() == ':') {
      if (token_lines[i].size() > 1) {
        throw Errors::Syntax_error("Extraneous tokens other than a label");
      }
      labels[i] = first_token.substr(0, first_token.length() - 1);
    }
  }

  for (auto it = labels.begin(); it != labels.end(); ++it) {
    for (std::size_t i{0}; i < token_lines.size(); ++i) {
      std::vector<std::string> &currrent_line{token_lines[i]};
      auto token = std::find(currrent_line.begin(), currrent_line.end(), it->second);
      if (token != currrent_line.end()) {
        const std::size_t label_addr{calculate_abs_addr(it, labels.end())};
        *token = std::to_string(static_cast<ptrdiff_t>(label_addr) - static_cast<ptrdiff_t>(i));
      }
    }
  }
}

inline void write_lines(const std::vector<std::vector<std::string>> token_lines) {
  auto &writer = std::cout;

  for (auto &line_el : token_lines) {

    std::copy(line_el.begin(), line_el.end(), std::ostream_iterator<std::string>{writer, " "});
    std::cout << "\n";
  }
}
