#ifndef CPP_TEMPLATES_VARIADIC_TEMPLATES
#define CPP_TEMPLATES_VARIADIC_TEMPLATES

#include <array>
#include <iostream>
#include <string>
#include <vector>

// This template gets preferred during overload resolution if trailing parameter
// pack is empty
template <typename Arg> void variadic_print(Arg &&arg) {
  std::cout << arg << '\n';
}

template <typename FirstArg, typename... Args>
void variadic_print(FirstArg &&first_arg, Args &&...args) {
  variadic_print(first_arg);
  variadic_print(std::forward<Args>(args)...);
}

template <typename T, char Delimiter = ' '> class AddDelimiter {
public:
  template <typename ValueType>
  explicit AddDelimiter(ValueType &&t) : value{std::move(t)} {}

  friend std::ostream &operator<<(std::ostream &os,
                                  const AddDelimiter<T> &add) {
    os << add.value << Delimiter;
    return os;
  }

private:
  T value;
};

// Variadic printing using fold expression
template <typename... Args> void variadic_print_line(Args &&...args) {
  // Same pattern (init op ... op args) where std::cout is initial value
  (std::cout << ... << AddDelimiter<Args>(std::forward<decltype(args)>(args)))
      << '\n';
}

// Non-type template parameter packs also work
template <typename std::size_t... Indices, typename Container>
void variadic_print_indices(const Container &c) {
  // Non-type template parameters are available implicitly
  variadic_print_line(c[Indices]...);
}

template <std::size_t... Indices> struct Index {};

template <typename Container, std::size_t... Indices>
void variadic_print_indices(const Container &c, const Index<Indices...> index) {
  // Works with containers that offer compile-time access using std::get
  variadic_print_line(std::get<Indices>(c)...);
}

template <typename... Args> constexpr auto size(Args &&...args) {
  // sizeof...(Args) also returns the parameter pack size (and not the number of
  // unique types)
  return sizeof...(args);
}

template <typename... Args> constexpr auto fold_and(Args &&...args) {
  return (... && args);
}

template <typename T, typename... Args>
constexpr auto fold_square_and_add(T &&initial_value, Args &&...args) {
  return (initial_value + ... + (args * args));
}

template <typename T, typename... Args>
constexpr auto fold_is_homogeneous(T &&t, Args &&...args) {
  return (std::is_same_v<T, Args> && ...);
}

// Binary tree node
struct Node {
  explicit Node(int i) noexcept : value{i}, left{nullptr}, right{nullptr} {}
  int value{0};
  Node *left;
  Node *right;
};

// Pointers to left and right members in node struct
inline auto left = &Node::left;
inline auto right = &Node::right;

template <typename RootNode, typename... Steps>
auto fold_traverse(RootNode &&root, Steps &&...steps) {
  return (root->*...->*steps);
}

// Mocked std::array-like container
template <typename T, std::size_t N> class MockedArray {
public:
  explicit MockedArray(std::initializer_list<T> &&init_list) {
    std::cout << "MockedArray<" << typeid(T).name() << ',' << N
              << ">::MockedArray()" << '\n';
  }
};

// Variadic Class Template Argument Deduction (CTAD) for the mocked array!
// Deduction guide for initializer-list constructor checks for consistent
// typing (T) and infers the size of the initializer-list (N) and resolves
// the specialized class template to be MockedArray<T, N>
template <typename T, typename... U>
MockedArray(T, U...)
    -> MockedArray<std::enable_if_t<(std::is_same_v<T, U> && ...), T>,
                   (1U + sizeof...(U))>;

#endif // !CPP_TEMPLATES_VARIADIC_TEMPLATES
