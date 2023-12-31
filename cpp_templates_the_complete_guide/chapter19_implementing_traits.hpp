#ifndef CPP_TEMPLATES_CHAPTER19_IMPLEMENTING_TRAITS
#define CPP_TEMPLATES_CHAPTER19_IMPLEMENTING_TRAITS

#include <iostream>
#include <iterator>
#include <tuple>
#include <type_traits>

// Helper to ignore any number of template parameters.
#ifndef __cpp_lib_void_t
namespace std {
template <typename...> using void_t = void;
}
#endif

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
 * that produces a value of type in unevaluated contexts without requiring type
 * to be default-constructible.
 * It is intentionally left undefined as it is only meant to be used in contexts
 * where no definition is needed (decltype, sizeof, ...).
 *
 * namespace std {
 * template <typename T>
 * std::add_rvalue_reference_t<T> declval() noexcept;
 * }
 */

template <typename, typename, typename = std::void_t<>>
struct HasPlusT : std::false_type {};

template <typename T1, typename T2>
struct HasPlusT<T1, T2,
                std::void_t<decltype(std::declval<T1>() + std::declval<T2>())>>
    : std::true_type {};

// Result type traits
template <typename T1, typename T2, bool = HasPlusT<T1, T2>::value>
struct PlusResultT {
  // Declval produces a value of type without requiring type to be
  // default-constructible.
  using Type = decltype(std::declval<T1>() + std::declval<T2>());
};
// Partial specialization that does not provide the Type member and is the
// default if there is no operator+() defined for mixing T1 and T2.
// This leads to functions for which this PlusResultT is invalid, i.e. does
// not provide the Type member, to be SFINAED-out.
template <typename T1, typename T2> struct PlusResultT<T1, T2, false> {};
template <typename T1, typename T2>
using PlusResult = typename PlusResultT<T1, T2>::Type;

template <typename T> class Array {};

// Nesting of traits to get the decayed type determined by PlusResult.
// This allows for sensible calls to the plus-operator when types T1 and T2 are
// different.
// This would get SFINAED-out if HasPlusT inside PlusResultT evaluates to false
// and therefore the alias PlusResult is invalid as it lacks the Type member.
template <typename T1, typename T2>
Array<RemoveReference<RemoveConstVolatile<PlusResult<T1, T2>>>>
operator+(const Array<T1> &, const Array<T2> &);

// We do this indirection via HasPlusT because a trait template should never
// fail at instantiation time if given reasonable template arguments as input.
// Therefore, it is common to perform the check twice:
// Once to find out whether the operation is valid (HasPlusT)
// Once to compute the result (PlusResultT)

// SFNIAE-out function overloads

template <typename T> struct IsUnimprovedDefaultConstructibleT {
private:
  // test() tries to substitute call of a default constructor for T passed as U.
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

// Primary template
// Second template argument enables us to provide partial specializations that
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
// lambda expression. This returned closure can be invoked with args and yields
// true_type or false_type based on which is_valid_impl is resolved.
// is_valid is a traits factory that generates traits-checking objects from its
// arguments.
inline constexpr auto is_valid = [](auto f) {
  // Returns a closure that can be invoked with args... and checks if f is
  // callable with args...
  return [](auto &&...args) {
    // true_type or false_type based on f being callable with args...
    return decltype(is_valid_impl<decltype(f), decltype(args) &&...>(
        nullptr)){};
  };
};

// Helper template to represent a type as value.
template <typename T> struct TypeT {
  using Type = T;
};
// Helper to wrap a type as a value by default-constructing the specialized
// helper template class TypeT.
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
// decltype(value_t(x))() tries to default-construct the type wrapped into x in
// an unevaluated context.
// is_default_constructible is a closure that can be evaluated by calling it
// with a single argument of type TypeT<T>.
constexpr auto is_default_constructible =
    is_valid([](auto x) -> decltype((void)decltype(value_t(x))()) {});
// Use like this: constexpr auto res = is_default_constructible(type<int>);

/**
 * Is expanded to:
 *
 * constexpr auto is_default_constructible = [](auto &&...args) {
 *  return decltype(is_valid_impl<
 *                 decltype([
 *                 ](auto x) -> decltype((void)decltype(value_t(x))())),
 *                 decltype(args) &&...>(nullptr)){};
 * };
 */

constexpr auto has_first =
    is_valid([](auto x) -> decltype((void)value_t(x).first) {});

// Detecting members

// Primary template.
template <typename, typename = std::void_t<>>
struct HasSizeTypeT : std::false_type {};
// Partial specialization that might be SFNIAEd-out. Also gets SFNIAEd-out when
// size_type is private.
template <typename T>
struct HasSizeTypeT<T, std::void_t<typename Decay<T>::size_type>>
    : std::true_type {};

// Detecting arbitrary properties of a class (like member types, member
// functions, ...) can be achieved by defining a macro like so.
#define DEFINE_HAS_TYPE(Member)                                                \
  template <typename, typename = std::void_t<>>                                \
  struct HasTypeT_##Member : std::false_type {};                               \
  template <typename T>                                                        \
  struct HasTypeT_##Member<T, std::void_t<typename Decay<T>::Member>>          \
      : std::true_type {};
// DEFINE_HAS_TYPE(size_type); if (HasTypeT_size_type<int>::value) ...;

// Due to the nature of std::void_t, we can combine multiple constraints into a
// single trait.
template <typename, typename = std::void_t<>>
struct IsIterableT : std::false_type {};
template <typename T>
struct IsIterableT<T, std::void_t<decltype(std::declval<T>().begin()),
                                  decltype(std::declval<T>().end()),
                                  decltype(std::declval<T>().cbegin()),
                                  decltype(std::declval<T>().cend())>>
    : std::true_type {};

// We can also use generic lambdas to detect arbitrary members since C++17
// Note that we do not have to take the indirection via value_t as return-type
// is now evaluated as part of the immediate context and SFINAE occurs.
constexpr auto has_size_type =
    is_valid([](auto &&x) -> typename std::decay_t<decltype(x)>::size_type {});
template <typename T>
using has_size_type_t = decltype(has_size_type(std::declval<T>()));
constexpr auto has_less =
    is_valid([](auto &&x, auto &&y) -> decltype(x < y) {});
template <typename T1, typename T2>
using has_less_t = decltype(has_less(std::declval<T1>(), std::declval<T2>()));
// std::cout << std::boolalpha << std::has_less_t<int, double>::value << '\n';

// If-Then-Else Traits (available as std::conditional<> in the Standard Library)

template <bool Cond, typename TrueT, typename FalseT> struct IfThenElseT {
  using Type = TrueT;
};
template <typename TrueT, typename FalseT>
struct IfThenElseT<false, TrueT, FalseT> {
  using Type = FalseT;
};
template <bool Cond, typename TrueT, typename FalseT>
using IfThenElse = typename IfThenElseT<Cond, TrueT, FalseT>::Type;

// Example: A trait that yields the corresponding unsigned type from the signed
// type. There exists std::make_unsigned but it requires that the type is a
// signed integral type and not bool. If these requirements are not met, using
// std::make_unsigned is undefined behavior.

// Template arguments for both the then (TrueT) and else (FalseT) branches are
// evaluated before the selection so they may not contain ill-formed code.
// Therefore, we wrap the desired TrueT and FalseT in type functions themselves.
template <typename T> struct IdentityT {
  using Type = T;
};
template <typename T> struct MakeUnsignedT {
  using Type = typename std::make_unsigned<T>::type;
};
// IfThenElse only selects the correct type function instance and only then the
// ::Type is evaluated for the selected type function instance.
template <typename T> struct UnsignedT {
  // First evaluates all template arguments, then selects the valid
  // specialization based on the first condition argument. Only then the ::Type
  // is applied.
  using Type =
      typename IfThenElse<std::is_integral_v<T> && !std::is_same_v<T, bool>,
                          MakeUnsignedT<T>, IdentityT<T>>
      // We need to apply the ::Type after selection!
      ::Type;

  /**
   * Also following would not even work as for T = bool, the ill-formed
   * expression MakeUnsignedT<bool>::Type would be evaluated before the
   * selection occurs and we get undefined behavior.
   *
   * using Type = typename
   * IfThenElse<std::is_integral_v<T> && !std::is_same_v<T, bool>,
   * MakeUnsignedT<T>::Type, T>;
   */
};

// Detecting non-throwing operations

// Primary template if move-construction is not valid
template <typename, typename = std::void_t<>>
struct IsNothrowMoveConstructibleT : std::false_type {};

// Specialization that might be SFINAEd-out if move-construction is not valid.
// Therefore, the noexcept expression will only be evaluated in contexts when
// move-construction is generally possible.
// decltype(T(std::declval<T>())) checks if T(T()), i.e. move-construction, is
// valid.
// Same logic from IfThenElse applies. First, the template arguments are
// evaluated. Only then will the evaluation of the noexcept expression take
// place iff the specialization for move-constructable types is selected.
template <typename T>
struct IsNothrowMoveConstructibleT<T,
                                   std::void_t<decltype(T(std::declval<T>()))>>
    : std::bool_constant<noexcept(T(std::declval<T>()))> {};

// Alias and variable templates reduce boilerpate (no typename and ::type or
// ::value required) but there are downsides:
//
// 1) Alias templates cannot be specialized and will likely need to redirect to
//    a class template anyway.
// 2) Some traits are meant to be specialized by the user (directly conflicting
//    with 1).
// 3) The use of the alias template will always instantiate the type which may
//    be undesired (see the IfThenElse example above).
//
// => Provide both class templates and alias/variable templates with a distinct
//    and consistent naming convention so the user may choose depending on the
//    use-case.

// Type classification

// Primary template.
template <typename T> struct IsFundamentalT : std::false_type {};
// Macro to specialize for fundamental types.
#define REGISTER_FUNDAMENTAL_TYPE(T)                                           \
  template <> struct IsFundamentalT<T> : std::true_type {};
// Use: REGISTER_FUNDAMENTAL_TYPE(bool);}

// For these compound types, one would usually define members that lets the user
// query attributes like base types, array dimensions, return types, ...
template <typename> struct IsPointerT : std::false_type {};
template <typename T> struct IsPointerT<T *> : std::true_type {};
template <typename> struct IsLValueReferenceT : std::false_type {};
template <typename T> struct IsLValueReferenceT<T &> : std::true_type {};
template <typename> struct IsRValueReferenceT : std::false_type {};
template <typename T> struct IsRValueReferenceT<T &&> : std::true_type {};
template <typename T>
// If lvalue reference, inherit from IsLValueReferenceT and get std::true_type
// via metafunction forwarding, else inherit from IsRValueReferenceT and get
// std::true_type if T is an rvalue reference or std::false_type otherwise.
struct IsReferenceT
    : IfThenElseT<IsLValueReferenceT<T>::value, IsLValueReferenceT<T>,
                  IsRValueReferenceT<T>>::Type {};
template <typename> struct IsArrayT : std::false_type {};
template <typename T, std::size_t N> struct IsArrayT<T[N]> : std::true_type {};
template <typename T> struct IsArrayT<T[]> : std::true_type {};
template <typename> struct IsPointerToMemberT : std::false_type {};
template <typename T, typename C>
struct IsPointerToMemberT<T C::*> : std::true_type {};
template <typename> struct IsFunctionT : std::false_type {};
template <typename ReturnType, typename... Args>
struct IsFunctionT<ReturnType(Args...)> : std::true_type {};
template <typename ReturnType, typename... Args>
struct IsFunctionT<ReturnType(Args..., ...)> : std::true_type {};
// Many more partial specializations for function types are required to
// correctly classify all combinations of const, volatile, lvalue and rvalue
// reference qualifiers.
template <typename, typename = std::void_t<>>
struct IsClassT : std::false_type {};
// Exploits the fact that only class types can be used as basis for
// pointer-to-member types. Note that the actual presence of an accessible
// member of type int is not required here and int is chosen arbitrarily.
// Lambda expressions are unnamed class types and hence would also yield true.
template <typename T>
struct IsClassT<T, std::void_t<int T::*>> : std::true_type {};
// Enumeration types are the only types not convered by the implemented type
// classification scheme yet.
template <typename T> struct IsEnumerationT {
  // clang-format off
  static constexpr bool value =
      !IsFundamentalT<T>::value &&
      !IsPointerT<T>::value &&
      !IsReferenceT<T>::value &&
      !IsArrayT<T>::value &&
      !IsPointerToMemberT<T>::value &&
      !IsFunctionT<T>::value &&
      !IsClassT<T>::value;
  // clang-format on
};

// So far, we only looked at property traits which can be used to determine the
// properties of template parameters. In contrast, policy traits define how some
// types should be treated. An example is the std::allocator trait that defines
// how memory allocation should be handled for the standard container types.

// In-depth policy trait example: Read-only parameter types

template <typename T> struct ReadParam {
  // If T is small and trivially move- and copy-constructible (i.e move or copy
  // operation is just a simple move or copy of the underlying bytes), passing
  // the parameter by value is fine. Else pass by const& because copy and move
  // are most likely too expensive.
  using Type = IfThenElse<sizeof(T) <= 2U * sizeof(void *) &&
                              std::is_trivially_copy_constructible_v<T> &&
                              std::is_trivially_move_constructible_v<T>,
                          T, const T &>;
};

template <typename... Args>
void bar_impl(typename ReadParam<Args...>::Type &&args) {
  // Do some work
}

// Forwarding to bar_impl() allows calling bar() with argument deduction, i.e.
// bar(5, 7.3) instead of bar<int, double>(5, 7.3), as the call without argument
// deduction is hidden in the forwarding-call of bar_impl().
// We pay for this cleaner interface with the overhead that comes with
// perfect-forwarding (assuming the compiler does not elide the inline function
// bar_impl()).
template <typename... Args> void bar(Args &&...param) {
  bar_impl<Args...>(std::forward<Args>(param)...);
}

#endif // !CPP_TEMPLATES_CHAPTER19_IMPLEMENTING_TRAITS
