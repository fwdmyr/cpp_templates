#ifndef CPP_TEMPLATES_CHAPTER20_OVERLOADING_ON_TYPE_PROPERTIES
#define CPP_TEMPLATES_CHAPTER20_OVERLOADING_ON_TYPE_PROPERTIES

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
EnableIf<IsRandomAccessIterator<Iter>>
advance_enable_impl(Iter &it, Distance n, std::random_access_iterator_tag) {
  it += n;
}
// The more general function overload.
template <typename Iter, typename Distance>
EnableIf<!IsRandomAccessIterator<Iter>>
advance_enable_impl(Iter &it, Distance n, std::input_iterator_tag) {
  while (n > 0) {
    it++;
    n--;
  }
}
// The desired overload resolution works because the EnableIf conditions are
// mutually exclusive.
template <typename Iter, typename Distance>
void advance_enable(Iter &it, Distance n) {
  advance_enable_impl(it, n,
                      typename std::iterator_traits<Iter>::iterator_category());
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

#endif // !CPP_TEMPLATES_CHAPTER20_OVERLOADING_ON_TYPE_PROPERTIES
