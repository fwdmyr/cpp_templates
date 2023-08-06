#ifndef CPP_TEMPLATES_CHAPTER6_ENABLE_IF
#define CPP_TEMPLATES_CHAPTER6_ENABLE_IF

#include <iostream>
#include <string>
#include <tuple>
#include <type_traits>

// Substitutes void return type for enable_if_t expression if true.
// Otherwise, enable_if_t is not defined (SFINAE)
template <typename T> std::enable_if_t<(sizeof(T) > 4U)> foo() {}
// Equivalent and more common syntax is this
template <typename T, typename = std::enable_if_t<(sizeof(T) > 4U)>>
void foo() {}

// Substitutes int return type for enable_if_t expression.
// Otherwise, enable_if_t is not defined (SFINAE)
template <typename T> std::enable_if_t<(sizeof(T) > 4U), std::size_t> foo() {
  return std::size_t{};
}
// Equivalent and more common syntax is this
template <typename T, typename = std::enable_if_t<(sizeof(T) > 4U)>>
std::size_t foo() {
  return std::size_t{};
}

class String {
public:
  // Member function templates are greedy and might lead to unexpected
  // invocations when their signature provides a better match than the special
  // member function. Without constraining the template types, this function is
  // a better match when called with a non-const String object since the
  // implicitly generated copy-constructor's argument is const String&. Only
  // passing a const String object will invoke the implicitly generated
  // copy-constructor.
  template <typename S, typename = std::enable_if_t<
                            std::is_constructible_v<std::string, S>>>
  String(S &&s);

private:
  std::string str{};
};

template <typename S, typename>
String::String(S &&s) : str{std::forward<S>(s)} {
  std::cout << "String::String(S &&s)" << std::endl;
}

#endif // !CPP_TEMPLATES_CHAPTER6_ENABLE_IF
