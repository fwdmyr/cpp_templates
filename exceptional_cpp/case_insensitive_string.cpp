#include "case_insensitive_string.hpp"
#include <cassert>
#include <cstring>
#include <iostream>

int main() {
  const auto s = ci_string{"AbCdE?"};

  assert(s == "abcde?");
  assert(s == "AbCdE?");
  assert(s != "abcdef?");
  assert(s != "AbCdEf?");

  assert(std::strcmp(s.c_str(), "abcde?") != 0);
  assert(std::strcmp(s.c_str(), "AbCdE?") == 0);
  assert(std::strcmp(s.c_str(), "abcdef?") != 0);
  assert(std::strcmp(s.c_str(), "AbCdEf?") != 0);

  return 0;
}
