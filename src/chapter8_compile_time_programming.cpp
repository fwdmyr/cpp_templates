#include "../include/chapter8_compile_time_programming.hpp"

int main() {
  std::ignore = cpp98::Alternative<69U>{};
  std::ignore = cpp98::Alternative<31U>{};
  std::ignore = cpp14::Alternative<69U>{};
  std::ignore = cpp14::Alternative<31U>{};

  cpp98::foo<69U>();
  cpp98::foo<31U>();
  cpp14::foo<69U>();
  cpp14::foo<31U>();

  std::string str = {"friend"};
  print(5.2, "hello", 69, 1.1, str);

  return 0;
}
