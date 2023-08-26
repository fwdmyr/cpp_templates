#ifndef CPP_TEMPLATES_CHAPTER11_GENERIC_LIBRARIES
#define CPP_TEMPLATES_CHAPTER11_GENERIC_LIBRARIES

#include <functional>
#include <iostream>
#include <memory>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

template <typename Iter, typename Callable>
void foreach (Iter current, Iter end, Callable op) {
  while (current != end) {
    op(*current);
    ++current;
  }
}

// Generalizes previous foreach...
template <typename Iter, typename Callable, typename... Args>
void foreach (Iter current, Iter end, Callable op, const Args &...args) {
  while (current != end) {
    // If op is a member function, the first arg is the this object.
    // Do not perfect-forward args as we may invoke multiple times.
    std::invoke(op, args..., *current);
    ++current;
  }
}

// Wrap a single function call and do some additional work (logging, measuring).
template <typename Callable, typename... Args>
// We use decltype(auto) as we want to return the exact return type.
decltype(auto) call(Callable &&op, Args &&...args) {
  // Initializing decltype(auto) ret as void is not allowed. We need to
  // distinguish between function calls that return void and non-void.
  if constexpr (std::is_same_v<std::invoke_result_t<Callable, Args...>, void>) {
    std::invoke(std::forward<Callable>(op), std::forward<Args>(args)...);
    // Do something...
    return;
  } else {
    decltype(auto) ret =
        std::invoke(std::forward<Callable>(op), std::forward<Args>(args)...);
    // Do something...
    return ret;
  }
}

template <typename T> void addr(T &&t) {
  // Might fail with overloaded operator&.
  auto p = &t;
  // Works always.
  auto q = std::addressof(t);
}

// Use declval as a placeholder for an object reference of type. This deduces
// the return type from the passed parameters without the need to
// default-construct objects of their types.
// Declval is only useable in unevaluated operands like decltype and we also
// should not forget to decay the rference it returns here.
template <typename T1, typename T2,
          typename TR = std::decay_t<decltype(true ? std::declval<T1>()
                                                   : std::declval<T2>())>>
TR max(T1 &&a, T2 &&b) {
  return b < a ? a : b;
}

// Use auto&& to create a variable that can be forwarded at a later stage and
// get its exact type with decltype to employ perfect-forwarding.
template <typename T> void foo(const T &t) {
  auto &&val = get(t);
  // Do something...
  set(std::forward<decltype(val)>(val));
}

inline void f(int i) { std::cout << "f(" << i << ")" << '\n'; }

struct F {
  // Const function-call operator!
  void operator()(int i) const {
    std::cout << "F::operator(" << i << ")" << '\n';
  }
};

struct C {
  // Const member function
  void memberfn(int i) const {
    std::cout << "C::memberfn(" << i << ")" << '\n';
  }
};

#endif // !CPP_TEMPLATES_CHAPTER11_GENERIC_LIBRARIES
