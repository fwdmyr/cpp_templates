#include "chapter6_enable_if.hpp"

int main() {
  auto str = std::string{"string"};
  // Calls template constructor
  std::ignore = String("tmp");
  // Calls template constructor
  std::ignore = String(str);
  // Calls template constructor
  auto str_object = String(std::move(str));
  // Calls implicitly generated copy-constructor
  std::ignore = String(str_object);
  // Calls implicitly generated move-constructor
  std::ignore = String(std::move(str_object));

  return 0;
}
