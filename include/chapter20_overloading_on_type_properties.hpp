#ifndef CPP_TEMPLATES_CHAPTER20_OVERLOADING_ON_TYPE_PROPERTIES
#define CPP_TEMPLATES_CHAPTER20_OVERLOADING_ON_TYPE_PROPERTIES

#include "chapter19_implementing_traits.hpp"
#include <iterator>

// Tag dispatching

/** Implementation of iterator tags in the Standard Library:
 *
 * namespace std {
 * struct input_iterator_tag {};
 * struct output_iterator_tag {};
 * struct forward_iterator_tag : public input_iterator_tag {};
 * struct bidirectional_iterator_tag : public forward_iterator_tag {};
 * struct random_access_iterator_tag : public bidirectional_iterator_tag {};
 * }
 */

// The more general function overload.
template <typename Iter, typename Distance>
void advance_dispatch_impl(Iter &it, Distance n, std::input_iterator_tag) {
  while (n > 0) {
    it++;
    n--;
  }
}
// The more specialized function overload as random_acces_iterator_tag inherits
// from input_iterator_tag.
// This will be preferred when called with random_access_iterator_tag.
template <typename Iter, typename Distance>
void advance_dispatch_impl(Iter &it, Distance n,
                           std::random_access_iterator_tag) {
  it += n;
}
// The desired overload resolution works because the tags have a clear
// inheritance history.
template <typename Iter, typename Distance>
void advance_dispatch(Iter &it, Distance n) {
  advance_dispatch_impl(
      it, n, typename std::iterator_traits<Iter>::iterator_category());
}

// Enabling and disabling function templates

template <bool, typename T = void> struct EnableIfT {};
template <typename T> struct EnableIfT<true, T> {
  using Type = T;
};
template <bool Cond, typename T = void>
using EnableIf = typename EnableIfT<Cond, T>::Type;

template <typename Iter>
constexpr auto IsRandomAccessIterator = std::is_convertible_v<
    typename std::iterator_traits<Iter>::iterator_category,
    std::random_access_iterator_tag>;
template <typename Iter>
constexpr auto IsInputIterator = std::is_convertible_v<
    typename std::iterator_traits<Iter>::iterator_category,
    std::input_iterator_tag>;

// This will be selected when called with a random access iterator.
// Note that the alias EnableIf is the return type of the function template. If
// the condition is true, EnableIf corresponds to its second template parameter
// (here void).
//
// What happens here is this:
//
// 1) Template argument IsRandomAccessIterator<Iter> is evaluated
// 2) Alias template EnableIf is evaluated. If its template argument (1) does
//    not have a Type member, i.e. the condition evaluated to false, this
//    overload gets SFINAEd-out. Otherwise, the alias template evaluates to
//    void and completes the function template declaration.
//
// => Essentially, EnableIf acts as a guard that protects against template
//    instantiations with types that violate the imposed constraints.
template <typename Iter, typename Distance>
EnableIf<IsRandomAccessIterator<Iter>> advance_enable_impl(Iter &it,
                                                           Distance n) {
  it += n;
}
// The more general function overload.
template <typename Iter, typename Distance>
EnableIf<!IsRandomAccessIterator<Iter>> advance_enable_impl(Iter &it,
                                                            Distance n) {
  while (n > 0) {
    it++;
    n--;
  }
}
// The desired overload resolution works because the EnableIf conditions are
// mutually exclusive.
template <typename Iter, typename Distance>
void advance_enable(Iter &it, Distance n) {
  advance_enable_impl(it, n);
}

// Another way to implement algorithm specialization is constexpr-if
template <typename Iter, typename Distance>
void advance_constexpr_if(Iter &it, Distance n) {
  if constexpr (IsRandomAccessIterator<Iter>) {
    it += n;
  } else {
    while (n > 0) {
      it++;
      n--;
    }
  }
}

// Note on enable-if algorithm specialization:
// Employing conditional enabling and disabling for robust algorithm
// specialization requires the condition for each function template overload to
// be mutually exclusive w.r.t. all the other conditions. This gets complicated
// pretty quickly as the number of specializations grows. Additionally, the
// conditions for all existing specializations must be revised when adding a new
// specialization.
// Tag dispatching does not suffer this problem but is restricted to less
// complicated conditions based on clear hierarchies.

// Note on constexpr-if algorithm specialization:
// The conditional code paths will only be instantiated for types that can
// support them, which is nice. This approach to specialization is not possible
// when different interfaces are involved or different class definitions are
// needed. It is also not possible to disable undesired function template
// instantiation for invalid template arguments.

// Typically, EnableIf is used in the function return type, but may also be used
// as an additional template parameter in special cases where no return type
// exists (e.g. constructor overloads, type conversion functions).
template <typename T> class Container {
public:
  // Construct from input iterator sequence.
  template <typename Iter, typename = EnableIf<IsInputIterator<Iter> &&
                                               !IsRandomAccessIterator<Iter>>>
  Container(Iter start, Iter end);
  // Construct from random access iterator sequence.
  template <typename Iter, typename = EnableIf<IsRandomAccessIterator<Iter>>,
            char> // Extra dummy template to avoid identical templates
  Container(Iter start, Iter end);

  // Conversion operator iff value types itself are convertible.
  template <typename U, typename = EnableIf<std::is_convertible_v<T, U>>>
  operator Container<U>() const;
};

// Class specialization

template <typename T, typename = std::void_t<>>
struct IsHashable : std::false_type {};
template <typename T>
struct IsHashable<
    T, std::void_t<decltype(std::declval<std::hash<T>>()(std::declval<T>()))>>
    : std::true_type {};
template <typename T> constexpr auto IsHashableV = IsHashable<T>::value;

// We do not need to disable anything on the primary template as partial
// specializations take precedence during matching.
// May implement an ordered map type container.
template <typename Key, typename Value, typename = void> class Dictionary {};

// Note that we would also need to ensure mutual exclusivity for the conditions
// of the partial specializations if there are multiple conditions.
// May implement an unordered map type container.
template <typename Key, typename Value>
class Dictionary<Key, Value, EnableIf<IsHashableV<Key>>> {};

// Tag dispatching for class templates is also possible. Emulating overload
// resolution for the partial class specializations (akin to advance_dispatch
// from above) can be done using function overload resolution.

// Instantiation-safe templates

// Instantiation-safe templates are templates that can never fail during
// instantiation. Instead they get SFINAEd-out during the deduction step. This
// can be achieved by encoding every operation that the template performs on its
// arguments as part of an EnableIf condition.

// Check if operator<(T1, T2) exists.
template <typename T1, typename T2, typename = std::void_t<>>
struct HasLess : std::false_type {};
template <typename T1, typename T2>
struct HasLess<T1, T2,
               std::void_t<decltype(std::declval<T1>() < std::declval<T2>())>>
    : std::true_type {};

// Deduce the return type of operator<(T1, T2).
template <typename T1, typename T2, bool HasLess> struct LessResultImpl {
  using Type = decltype(std::declval<T1>() < std::declval<T2>());
};
template <typename T1, typename T2> struct LessResultImpl<T1, T2, false> {};
template <typename T1, typename T2>
struct LessResultT : public LessResultImpl<T1, T2, HasLess<T1, T2>::value> {};

// We do this two-step process (check if operation exists, determine return
// type) to provide a SFINAE-friendly trait. This is a trait that should not
// fail to instantiate given reasonable arguments. More information can be found
// in the discussion of PlusResult in Chapter 19.
template <typename T1, typename T2>
using LessResult = typename LessResultT<T1, T2>::Type;

// Instantiation-safe min().
template <typename T>
EnableIf<std::is_convertible_v<LessResult<const T &, const T &>, const T &>,
         bool>
min(const T &lhs, const T &rhs) {
  return (rhs < lhs) ? rhs : lhs;
}

#endif // !CPP_TEMPLATES_CHAPTER20_OVERLOADING_ON_TYPE_PROPERTIES
