#ifndef CPP_TEMPLATES_CHAPTER19_IMPLEMENTING_TRAITS
#define CPP_TEMPLATES_CHAPTER19_IMPLEMENTING_TRAITS

#include <iostream>
#include <iterator>
#include <tuple>
#include <type_traits>

/**
 * Possible Standard Library implementation for iterator_traits pointer
 * specialization that enables our implementation of the accum function.
 *
 * namespace std {
 * template <typename T>
 * struct iterator_traits<T*> {
 * using difference_type = std::ptrdiff_t;
 * using value_type = T;
 * using pointer = T*;
 * using reference = T&;
 * using iterator_category = std::random_access_iterator_tag;
 * };
 * }
 */

template <typename T> struct AccumulationTraits {};

template <> struct AccumulationTraits<char> {
  using AccT = unsigned int;
  // Or use static member-function for non-primitive types
  static constexpr AccT zero = 0;
};

template <> struct AccumulationTraits<long> {
  using AccT = long;
  static constexpr AccT zero = 0;
};

// Policy that defines what accumulation means in this context.
struct SumPolicy {
  template <typename T1, typename T2>
  static void accumulate(T1 &total, const T2 &value) {
    total += value;
  }
};

// Adding traits as template parameter and providing default value allows the
// user to substitute in custom traits when needed.
// Adding policy as template parameter lets the user substitute any policy that
// implements the required interface.
// We order the template parameters in decreasing likelihood that they will be
// replaced by client code that instantiates the function template.
// Here the policy is more likely to differ from the default policy than traits
// from the default traits.
template <typename Iter, typename Policy = SumPolicy,
          typename Traits = AccumulationTraits<
              typename std::iterator_traits<Iter>::value_type>>
auto accum(Iter start, Iter end) {
  auto total = Traits::zero;
  while (start != end) {
    Policy::accumulate(total, *start);
    ++start;
  }
  return total;
}

template <typename T1, typename T2> struct SumPolicyTemplate {
  static void accumulate(T1 &total, const T2 &value) { total += value; }

private:
  // Internal state that depends on type parameters.
  T1 x;
};

// Policy templates allow to store and carry internal state that depends on its
// type parameters more easily. Policy templates lead to more complex and
// verbose function template signatures that require template template
// parameters.
template <typename Iter,
          template <typename, typename> class Policy = SumPolicyTemplate,
          typename Traits = AccumulationTraits<
              typename std::iterator_traits<Iter>::value_type>>
auto accum(Iter start, Iter end) {
  using ValueT = typename std::iterator_traits<Iter>::value_type;
  using AccT = typename Traits::AccT;

  auto total = Traits::zero;
  while (start != end) {
    Policy<AccT, ValueT>::accumulate(total, *start);
    ++start;
  }
  return total;
}

// Removing references
// Primary template that handles the default case, i.e. nothing to do
template <typename T> struct RemoveReferenceT {
  using Type = T;
};
// Specializations for lvalue and rvalue references
template <typename T> struct RemoveReferenceT<T &> {
  using Type = T;
};
template <typename T> struct RemoveReferenceT<T &&> {
  using Type = T;
};

// Removing const qualifier
template <typename T> struct RemoveConstT {
  using Type = T;
};
template <typename T> struct RemoveConstT<const T> {
  using Type = T;
};

// Removing volatile qualifier
template <typename T> struct RemoveVolatileT {
  using Type = T;
};
template <typename T> struct RemoveVolatileT<volatile T> {
  using Type = T;
};

// Metafunction forwarding to inherit the type member from RemoveConstT
template <typename T>
struct RemoveConstVolatileT : RemoveConstT<typename RemoveVolatileT<T>::Type> {
};
template <typename T>
using RemoveConstVolatile = typename RemoveConstVolatileT<T>::Type;

// The primary template class for type decay
template <typename T> struct DecayT : RemoveConstVolatileT<T> {};

// Handle array-to-pointer decay with and without bounds
template <typename T> struct DecayT<T[]> {
  using Type = T *;
};
template <typename T, std::size_t N> struct DecayT<T[N]> {
  using Type = T *;
};

// Handle function-to-pointer decay
template <typename ReturnType, typename... Args>
struct DecayT<ReturnType(Args...)> {
  using Type = ReturnType (*)(Args...);
};
// Special case: function type that uses C-style varargs
template <typename ReturnType, typename... Args>
struct DecayT<ReturnType(Args..., ...)> {
  using Type = ReturnType (*)(Args..., ...);
};
template <typename T> using Decay = typename DecayT<T>::Type;

// True and false types
template <bool val> struct BoolConstant {
  using Type = BoolConstant<val>;
  static constexpr bool value = val;
};
using TrueType = BoolConstant<true>;
using FalseType = BoolConstant<false>;

// Predicate traits using metafunction forwarding
// Primary template that handles the default case and inherits from false type
template <typename T1, typename T2> struct IsSameT : FalseType {};
// Specialization for same types that inherits from true type instead
template <typename T> struct IsSameT<T, T> : TrueType {};

// Tag dispatch using true and false types
// Is invoked if T is int
template <typename T> void foo_impl(T &&, TrueType);
// Is invoked otherwise
template <typename T> void foo_impl(T &&, FalseType);
// Function template that forwards the call to the implementations based on the
// predicate evaluation
template <typename T> void foo(T &&t) {
  foo_impl(std::forward<T>(t), IsSameT<Decay<T>, int>{});
}

// TODO: Continue here

// Result type traits
template <typename T1, typename T2> struct PlusResultT {
  // Declval produces a value of type without requiring type to be
  // default-constructible.
  using Type = decltype(std::declval<T1>() + std::declval<T2>());
};
template <typename T1, typename T2>
using PlusResult = typename PlusResultT<T1, T2>::Type;

#endif // !CPP_TEMPLATES_CHAPTER19_IMPLEMENTING_TRAITS
