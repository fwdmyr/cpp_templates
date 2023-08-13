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

#endif // !CPP_TEMPLATES_CHAPTER19_IMPLEMENTING_TRAITS
