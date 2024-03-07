#pragma once

#include "exception.hpp"
#include "cobra_algos.hpp"

#include <iterator>
#include <vector>
#include <string>
#include <sstream>
#include <map>
#include <algorithm>
#include <iostream>

using Line      = std::vector<std::string>;
using Line_addr = std::vector<Line>::iterator;

inline std::vector<std::string> line2tokens(const std::string &line) {
  std::stringstream ss{line};
  std::vector<std::string> tokens{};

  std::copy(std::istream_iterator<std::string>(ss),
      std::istream_iterator<std::string>(),
      std::back_inserter(tokens));

  return tokens;
}

inline Line_addr calculate_next_instr_addr(std::map<Line_addr, std::string_view>::const_iterator label_it,
    std::map<Line_addr, std::string_view>::const_iterator labels_end_it, Line_addr lines_end_it) {
  Line_addr current_line{label_it->first + 1};
  while (true) {
    // if a line is a label then skip
    if ((next(label_it) != labels_end_it) &&
        (next(label_it)->first == current_line)) {
      ++label_it;
      ++current_line;
    } else if (current_line == lines_end_it) {
      throw Errors::Syntax_error{"Invalid label's position"};
    } else if (is_empty_line(*current_line)) {
      ++current_line;
    } else {
      return current_line;
    }
  }
}

inline std::map<Line_addr, std::string_view> find_labels(std::vector<Line> &token_lines) {
  std::map<Line_addr, std::string_view> labels;
  for (Line_addr it = token_lines.begin(); it != token_lines.end(); ++it) {
    if (is_empty_line(*it)) continue;
    const std::string &first_token{(*it)[0]};
    if (first_token.back() == ':') {
      if ((*it).size() > 1) {
        throw Errors::Syntax_error("Extraneous tokens other than a label");
      }
      labels[it] = {cbegin(first_token), cend(first_token) - 1};
    }
  }

  return labels;
}

inline std::vector<Line_addr> remove_labels(std::vector<Line> &lines,
    const std::map<Line_addr, std::string_view> &labels) {
  std::vector<Line_addr> removed_labels{};
  for (Line_addr it = lines.begin(); it != lines.end(); ++it) {
    if (labels.find(it) == labels.end()) {
      removed_labels.push_back(it);
    }
  }

  return removed_labels;
}

inline std::vector<Line_addr> remove_empty_lines(std::vector<Line_addr> lines) {
  auto it = std::remove_if(lines.begin(), lines.end(), [](Line_addr el){
      return is_empty_line(*el);
  });
  std::vector<Line_addr> clear_lines{lines.begin(), it};

  return clear_lines;
}

inline void handle_labels(std::vector<std::vector<std::string>> &token_lines) {
  auto labels = find_labels(token_lines);
  std::map<Line_addr, std::string_view> addr_labels{};

  for (auto it = labels.begin(); it != labels.end(); ++it) {
    auto addr = calculate_next_instr_addr(it,
        labels.end(), token_lines.end());
    addr_labels[addr] = it->second;
  }

  auto clear_lines = remove_empty_lines(remove_labels(token_lines, labels));

  for (Line_addr &line_addr : clear_lines) {
    for (auto &label : addr_labels) {
      Line::iterator token = std::find(line_addr->begin(), line_addr->end(), label.second);
      auto used_label = label;
      if (token != line_addr->end()) {
        Line_addr label_addr{used_label.first};
        *token = std::to_string(label_addr - line_addr);
      }
    }

  }

  // for (auto it = labels.begin(); it != labels.end(); ++it) {
  //   for (std::size_t i{0}; i < token_lines.size(); ++i) {
  //     std::vector<std::string> &currrent_line{token_lines[i]};
  //     auto token = std::find(currrent_line.begin(), currrent_line.end(), it->second);
  //     if (token != currrent_line.end()) {
  //       const std::size_t label_addr{calculate_abs_addr(it, labels.end())};
  //       auto curr_it = token_lines.begin() + static_cast<ptrdiff_t>(i);
  //       auto label_it = token_lines.begin() + static_cast<ptrdiff_t>(label_addr);
  //       const ptrdiff_t empty_num{count_if(
  //           min(curr_it, label_it), max(curr_it, label_it), [](auto el){return el.empty();})
  //       };
  //       ptrdiff_t diff{static_cast<ptrdiff_t>(label_addr) - static_cast<ptrdiff_t>(i)};
  //       diff += (diff < 0) ? (empty_num + 1) : -empty_num;
  //       *token = std::to_string(diff);
  //     }
  //   }
  // }
}

inline void write_lines(const std::vector<std::vector<std::string>> token_lines) {
  auto &writer = std::cout;

  for (auto &line_el : token_lines) {

    std::copy(line_el.begin(), line_el.end(), std::ostream_iterator<std::string>{writer, " "});
    std::cout << "\n";
  }
}
