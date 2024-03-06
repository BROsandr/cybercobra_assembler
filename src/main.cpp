#include "compiler.hpp"
#include "reader.hpp"

#include <iostream>

int main() {
  Reader reader{std::cin};
  auto &writer = std::cout;

  compile(reader, writer, writer);

  return 0;
}
