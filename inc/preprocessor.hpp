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
using Labels    = std::map<std::string_view, Line_addr>;

inline std::vector<std::string> line2tokens(const std::string &line) {
  std::stringstream ss{line};
  std::vector<std::string> tokens{};

  std::copy(std::istream_iterator<std::string>(ss),
      std::istream_iterator<std::string>(),
      std::back_inserter(tokens));

  return tokens;
}

inline Line_addr calculate_next_instr_addr(Labels::const_iterator label_it,
    Labels::const_iterator labels_end_it, Line_addr lines_end_it) {
  Line_addr current_line{label_it->second + 1};
  while (true) {
    // if a line is a label then skip
    if ((next(label_it) != labels_end_it) &&
        (next(label_it)->second == current_line)) {
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

inline Labels find_labels(std::vector<Line> &token_lines) {
  Labels labels;
  for (Line_addr it = token_lines.begin(); it != token_lines.end(); ++it) {
    if (is_empty_line(*it)) continue;
    const std::string &first_token{(*it)[0]};
    if (first_token.back() == ':') {
      std::string_view label{cbegin(first_token), cend(first_token) - 1};
      if ((*it).size() > 1) {
        throw Errors::Syntax_error("Extraneous tokens other than a label");
      } else if (labels.contains(label)) {
        throw Errors::Syntax_error{"Label==" + std::string{label} + " already exists"};
      }
      labels[label] = it;
    }
  }

  return labels;
}

inline std::vector<Line_addr> remove_labels(std::vector<Line> &lines, const Labels &labels) {
  std::vector<Line_addr> not_labels{};
  for (Line_addr it = lines.begin(); it != lines.end(); ++it) {
    if (std::find_if(labels.begin(), labels.end(), [it](auto &label){ return label.second == it; }) == labels.end()) {
      not_labels.push_back(it);
    }
  }

  return not_labels;
}

inline std::vector<Line_addr> remove_empty_lines(std::vector<Line_addr> lines) {
  auto it = std::remove_if(lines.begin(), lines.end(), [](Line_addr el){
      return is_empty_line(*el);
  });
  std::vector<Line_addr> clear_lines{lines.begin(), it};

  return clear_lines;
}

inline void handle_labels(std::vector<Line> &token_lines) {
  auto labels = find_labels(token_lines);
  Labels addr_labels{};

  for (auto it = labels.begin(); it != labels.end(); ++it) {
    auto addr = calculate_next_instr_addr(it,
        labels.end(), token_lines.end());
    addr_labels[it->first] = addr;
  }

  auto no_labels = remove_labels(token_lines, labels);

  auto clear_lines = remove_empty_lines(no_labels);

  for (Line_addr &line_addr : clear_lines) {
    for (auto &label : addr_labels) {
      Line::iterator token = std::find(line_addr->begin(), line_addr->end(), label.first);
      auto used_label = label;
      if (token != line_addr->end()) {
        auto clear_label_addr = std::find(clear_lines.begin(), clear_lines.end(), used_label.second) - clear_lines.begin();
        auto clear_line_addr = std::find(clear_lines.begin(), clear_lines.end(), line_addr) - clear_lines.begin();
        *token = std::to_string(clear_label_addr - clear_line_addr);
      }
    }

  }

  for (auto it = token_lines.begin(); it != token_lines.end(); ++it) {
    if (std::find_if(no_labels.begin(), no_labels.end(),
            [it](Line_addr line_addr){ return line_addr == it; }) ==
        no_labels.end()) {
      *it = Line{""};
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

inline void preprocess(std::vector<Line> &token_lines) {
  handle_labels(token_lines);
}
