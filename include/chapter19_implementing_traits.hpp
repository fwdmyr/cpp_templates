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
template <typename T>
using RemoveReference = typename RemoveReferenceT<T>::Type;

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

/**
 * Possible Standard Library implementation for declval
 * that produces a value of type without requiring type to be
 * default-constructible.
 * It is intentionally left undefined as it is only meant to be used in contexts
 * where no definition is needed (decltype, sizeof, ...).
 *
 * namespace std {
 * template <typename T>
 * std::add_rvalue_reference_t<T> declval() noexcept;
 * }
 */

// Result type traits
template <typename T1, typename T2> struct PlusResultT {
  // Declval produces a value of type without requiring type to be
  // default-constructible.
  using Type = decltype(std::declval<T1>() + std::declval<T2>());
};
template <typename T1, typename T2>
using PlusResult = typename PlusResultT<T1, T2>::Type;

template <typename T> class Array {};

// Nesting of traits to get the decayed type determined by PlusResult.
// This allows for sensible calls to the plus-operator when types T1 and T2 are
// different.
template <typename T1, typename T2>
Array<RemoveReference<RemoveConstVolatile<PlusResult<T1, T2>>>>
operator+(const Array<T1> &, const Array<T2> &);

// SFNIAE-out function overloads

template <typename T> struct IsUnimprovedDefaultConstructibleT {
private:
  // test() trying substitute call of a default constructor for T passed as U.
  // Matches iff requested check succeeds. Note that the second template
  // parameter cannot be deduced from function arguments.
  // We cannot directly pass T here as for any T, all member functions are
  // substituted so the code would not compile if T is not
  // default-constructible.
  template <typename U, typename = decltype(U())> static char test(void *);
  // test() fallback.
  // Matches with ellipsis so other matches are always preferred.
  template <typename> static long test(...);

public:
  // Depends on which overloaded test() is selected.
  // Returns true if test() returns char, i.e. T is default-constructible.
  // Returns false otherwise.
  static constexpr bool value =
      IsSameT<decltype(test<T>(nullptr)), char>::value;
};

// Improved IsDefaultConstructibleT using true and false type.
template <typename T> struct IsDefaultConstructibleHelper {
private:
  // test() trying substitute call of a default constructor for T passed as U.
  template <typename U, typename = decltype(U())>
  static std::true_type test(void *);
  // test() fallback.
  template <typename> static std::false_type test(...);

public:
  // decltype so we do not need the definitions for test().
  using Type = decltype(test<T>(nullptr));
};
// Inherits from true_type iff T is default-constructible and false_type
// otherwise.
template <typename T>
struct IsDefaultConstructibleT : IsDefaultConstructibleHelper<T>::Type {};

// SFNIAE-out partial specializations

// Helper to ignore any number of template parameters.
#ifndef __cpp_lib_void_t
namespace std {
template <typename...> using void_t = void;
}
#endif

// Primary template
// Second template argument enables us th provide partial specializations that
// use an arbitrary number of compile-time constructs.
template <typename, typename = std::void_t<>>
struct IsDefaultConstructibleT2 : std::false_type {};

// Partial specialization as the constraint for the predicate that might be
// SFNIAEd-out.
// Here, we need a single construct decltype(T()) to determine if our type is
// indeed default-constructible.
template <typename T>
struct IsDefaultConstructibleT2<T, std::void_t<decltype(T())>>
    : std::true_type {};

// The approach for IsDefaultConstructibleT2 is more condensed than the first
// approach that overloads function templates but has the restriction that the
// condition (here decltype(T())) needs to be formulated inside the declaration
// of a template parameter.

// Since C++17, we can specify the condition in a generic lambda to minimize
// boilerplate code.

// Helper checking validity of f(args...) for F f and Args... args.
template <typename F, typename... Args,
          typename = decltype(std::declval<F>()(std::declval<Args &&>()...))>
std::true_type is_valid_impl(void *);
// Fallback if helper is SFINAEd-out.
template <typename F, typename... Args> std::false_type is_valid_impl(...);

// Define a generic lambda that takes a lambda f and returns whether calling f
// with args is valid.
// is_valid is a closure that itself returns a closure produced by the inner
// lambda expression.
// is_valid is a traits factory that generates traits-checking objects from its
// arguments.
inline constexpr auto is_valid = [](auto f) {
  return [](auto &&...args) {
    return decltype(is_valid_impl<decltype(f), decltype(args) &&...>(
        nullptr)){};
  };
};

// Helper template to represent a type as value.
template <typename T> struct TypeT {
  using Type = T;
};

// Helper to wrap a type as a value by default-constructing it.
template <typename T> constexpr auto type = TypeT<T>{};
// Helper to unwrap a wrapped type in unevaluated contexts by passing the
// wrapper to value_t.
template <typename T> T value_t(TypeT<T>);

/**
 * Wrap type into a value.
 * constexpr auto x = type<int>;
 * Unwrap value into type (in unevaluated context)
 * decltype(value_t(x))
 */

// Typical use-case of generic lambda SFINAE.
// decltype(value_t(x))() tries to default-construct the type wrapped into x.
constexpr auto is_default_constructible =
    is_valid([](auto x) -> decltype((void)decltype(value_t(x))()) {});

/**
 * Is expanded to :
 *
 * constexpr auto is_default_constructible2 = [](auto &&...args) {
 *  return decltype(is_valid_impl<
 *                 decltype([
 *                 ](auto x) -> decltype((void)decltype(value_t(x))())),
 *                 decltype(args) &&...>(nullptr)){};
 * };
 */

constexpr auto has_first =
    is_valid([](auto x) -> decltype((void)value_t(x).first) {});

#endif // !CPP_TEMPLATES_CHAPTER19_IMPLEMENTING_TRAITS
