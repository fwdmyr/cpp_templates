#include "../include/chapter11_generic_libraries.hpp"

int main() {
  const auto fibonacci = std::vector<int>{1, 1, 2, 3, 5, 8, 13, 21};

  // Pass function that decays to function pointer.
  foreach (fibonacci.begin(), fibonacci.end(), f)
    ;
  // Pass function pointer explicitly.
  foreach (fibonacci.begin(), fibonacci.end(), &f)
    ;
  // Pass functor that is invoked as op.operator(*current).
  foreach (fibonacci.begin(), fibonacci.end(), F())
    ;
  // Pass closure, i.e. a functor.
  foreach (fibonacci.begin(), fibonacci.end(),
           [](int i) { std::cout << "lambda(" << i << ")" << '\n'; })
    ;
  // Pass member function pointer.
  // Fourth argument is passed as second argument (after the function pointer)
  // to invoke.
  const auto c = C{};
  foreach (fibonacci.begin(), fibonacci.end(), &C::memberfn, c)
    ;
  // Pass closure, i.e. a functor, with additional arguments.
  // Fourth argument is passed as first argument to the closure.
  foreach (
      fibonacci.begin(), fibonacci.end(),
      [](const std::string &prefix, int i) {
        std::cout << prefix << "lambda(" << i << ")" << '\n';
      },
      "fib - ")
    ;

  return 0;
}
