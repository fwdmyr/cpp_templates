#ifndef CPP_TEMPLATES_CHAPTER_COMPILE_TIME_PROGRAMMING
#define CPP_TEMPLATES_CHAPTER_COMPILE_TIME_PROGRAMMING

#include <iostream>
#include <tuple>
#include <type_traits>

// C++98 compile-time is_prime
namespace cpp98 {

// Recursive check if P is integer-divisible by D (i.e. non-prime)
template <std::size_t P, std::size_t D> struct do_is_prime {
  static constexpr bool value = (P % D != 0U) && do_is_prime<P, D - 1U>::value;
};

// Recursive base case
template <std::size_t P> struct do_is_prime<P, 2U> {
  static constexpr bool value = (P % 2U != 0U);
};

template <std::size_t P> struct is_prime {
  static constexpr bool value = do_is_prime<P, P / 2U>::value;
};

// Special cases to avoid infinite recursion
template <> struct is_prime<0U> {
  static constexpr bool value = false;
};
template <> struct is_prime<1U> {
  static constexpr bool value = false;
};
template <> struct is_prime<2U> {
  static constexpr bool value = true;
};
template <> struct is_prime<3U> {
  static constexpr bool value = true;
};

template <std::size_t N, bool = is_prime<N>::value> struct Alternative;

// Partial specializations depending on whether N is prime or not
template <std::size_t N> struct Alternative<N, true> {
  Alternative() {
    std::cout << "Alternative<" << N << ", true>::Alternative()" << std::endl;
  }
};
template <std::size_t N> struct Alternative<N, false> {
  Alternative() {
    std::cout << "Alternative<" << N << ", false>::Alternative()" << std::endl;
  }
};

template <std::size_t N> void foo() {
  std::cout << N << " is ";
  if constexpr (!is_prime<N>::value) {
    std::cout << "not ";
  }
  std::cout << "prime" << '\n';
}

} // namespace cpp98

// C++14 compile-time is_prime
namespace cpp14 {

constexpr bool is_prime(std::size_t p) {
  for (std::size_t d = 2U; d <= p / 2U; ++d) {
    if (p % d == 0U) {
      return false;
    }
  }
  return p > 1;
}

template <std::size_t N, bool = is_prime(N)> struct Alternative;

// Partial specializations depending on whether N is prime or not
template <std::size_t N> struct Alternative<N, true> {
  Alternative() {
    std::cout << "Alternative<" << N << ", true>::Alternative()" << std::endl;
  }
};
template <std::size_t N> struct Alternative<N, false> {
  Alternative() {
    std::cout << "Alternative<" << N << ", false>::Alternative()" << std::endl;
  }
};

template <std::size_t N> void foo() {
  std::cout << N << " is ";
  if constexpr (!is_prime(N)) {
    std::cout << "not ";
  }
  std::cout << "prime" << '\n';
}

} // namespace cpp14

template <typename FirstArg, typename... Args>
void print(FirstArg &&first_arg, Args &&...args) {
  std::cout << first_arg << ' ';
  if constexpr (sizeof...(args) > 0U) {
    print(std::forward<Args>(args)...);
  } else {
    std::cout << '\n';
  }
}

// Substitution failure for anything besides raw arrays and string literals.
template <typename T, std::size_t N> std::size_t len(T (&)[N]) { return N; }

// Substitution failure for types that do not have a size_type alias and size()
// member function.
template <typename T> typename T::size_type len(const T &t) { return t.size(); }

// Fallback len() function if no other function template matches. In practice,
// this could throw an exception or contain a static_assert to provide a good
// compile-time error message.
inline std::size_t len(...) { return std::size_t{}; }

// But there is a problem: std::allocator<int> has a size_type alias but no
// size() member function. During compile-time, the overload returning the
// size_type is chosen over the fallback as its signature is the better match.
// This results in an error at a later compilation stage because size() is not
// available.
// This issue can be remedied by the following idiom using trailing
// return type syntax. Here the decltype operand is a comma-separated list of
// expressions, s.t. the last expression (here T::size_type()) yields the
// desired return type. Before the last comma are the expressions that must be
// valid (here t.size()) and the cast to void is to protect against overloaded
// operator,() for the expression type.
template <typename T>
auto safe_len(const T &t) -> decltype((void)(t.size)(), T::size_type()) {
  return t.size();
}

inline std::size_t safe_len(...) { return std::size_t{}; }

#endif // !CPP_TEMPLATES_CHAPTER_COMPILE_TIME_PROGRAMMING
