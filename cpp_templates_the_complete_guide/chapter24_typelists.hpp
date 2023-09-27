#ifndef CPP_TEMPLATES_CHAPTER24_TYPELISTS
#define CPP_TEMPLATES_CHAPTER24_TYPELISTS

#include "chapter19_implementing_traits.hpp"
#include <cstddef>

// Typelist implementation.

template <typename... Elements> class Typelist {};

// Get the front type of the typelist.
template <typename List> class FrontT;
template <typename Head, typename... Tail>
class FrontT<Typelist<Head, Tail...>> {
public:
  using Type = Head;
};
template <typename List> using Front = typename FrontT<List>::Type;

// Pop from the front the typelist and return the remaining typelist.
template <typename List> class PopFrontT;
template <typename Head, typename... Tail>
class PopFrontT<Typelist<Head, Tail...>> {
public:
  using Type = Typelist<Tail...>;
};
template <typename List> using PopFront = typename PopFrontT<List>::Type;

// Push to the front of the typelist and return the extended typelist.
template <typename List, typename NewElement> class PushFrontT;
template <typename... Elements, typename NewElement>
class PushFrontT<Typelist<Elements...>, NewElement> {
public:
  using Type = Typelist<NewElement, Elements...>;
};
template <typename List, typename NewElement>
using PushFront = typename PushFrontT<List, NewElement>::Type;

// Get the type at index N.
// Recursive case. Walk through the list and pop the N first types.
template <typename List, unsigned N>
class NthElementT : public NthElementT<PopFront<List>, N - 1> {};
// Base case. Inherit from FrontT which provides the front type alias.
template <typename List> class NthElementT<List, 0> : public FrontT<List> {};
template <typename List, unsigned N>
using NthElement = typename NthElementT<List, N>::Type;

// Check if the type list is empty.
template <typename List> struct IsEmpty {
  static constexpr bool value = false;
};
template <> struct IsEmpty<Typelist<>> {
  static constexpr bool value = true;
};

// Find the largest type in the typelist.
template <typename List, bool Empty = IsEmpty<List>::value> class LargestTypeT;
// Recursive case. Uses the First-Rest-Idiom.
template <typename List> class LargestTypeT<List, false> {
private:
  // 1) Compute partial result based on the first element.
  using First = Front<List>;
  // 2) Compute result for the rest of the elements in the list by recursing.
  using Rest = typename LargestTypeT<PopFront<List>>::Type;

public:
  // 3) Combine First and Rest to pick the solution, i.e. larger type of either
  // the first element or the best candidate so far encountered in the rest.
  using Type = IfThenElse<(sizeof(First) >= sizeof(Rest)), First, Rest>;
};
// Base case. Recursion stops when the list is empty. Use char by default as
// every other type is at least as large as char.
template <typename List> class LargestTypeT<List, true> {
public:
  using Type = char;
};
template <typename List> using LargestType = typename LargestTypeT<List>::Type;

// clang-format off
// Example (Typelist<bool, int, long long, short>):
// When arriving at the base case, we get: 
// Typelist<short>:                       First = short,     Rest = char      -> Type = short 
// Then we unwind the recursion step by step: 
// Typelist<long long, short>:            First = long long, Rest = short     -> Type = long long 
// Typelist<int, long long, short>:       First = int,       Rest = long long -> Type = long long
// Typelist<bool, int, long long, short>: First = bool,      Rest = long long -> Type = long long
// clang-format on

// Push to the back of the typelist and return the extended typelist.
template <typename List, typename NewElement, bool = IsEmpty<List>::value>
class PushBackRecT {};
// Recursive case. Again the First-Rest-Idiom.
template <typename List, typename NewElement>
class PushBackRecT<List, NewElement, false> {
private:
  using Head = Front<List>;
  using Tail = PopFront<List>;
  using NewTail = typename PushBackRecT<Tail, NewElement>::Type;

public:
  // Reattaches the head to the extended tail of the typelist.
  using Type = PushFront<NewTail, Head>;
};
// Base case. Push front and back are the same operation on an empty typelist.
template <typename List, typename NewElement>
class PushBackRecT<List, NewElement, true> {
public:
  using Type = PushFront<List, NewElement>;
};
// Generic push-back operation.
template <typename List, typename NewElement>
class PushBackT : public PushBackRecT<List, NewElement> {};
template <typename List, typename NewElement>
using PushBack = typename PushBackT<List, NewElement>::Type;

// clang-format off
// Example (push back bool to Typelist<int, long>):
// When arriving at the base case, we get: 
// Typelist<>:                                                                Type = <bool>
// Then we unwind the recursion step by step: 
// Typelist<long>:      Head = long, Tail = <>,     NewTail = <bool>       -> Type = <long, bool>
// Typelist<int, long>: Head = int,  Tail = <long>, NewTail = <long, bool> -> Type = <int, long, bool>
// clang-format on

// Reverse the typelist.
template <typename List, bool = IsEmpty<List>::value> class ReverseT;
template <typename List> using Reverse = typename ReverseT<List>::Type;
// Recursive case. Separates the head from the tail of the list. Recursively
// reverses the tail of the list and finally pushes the head to the back of the
// reversed tail.
template <typename List>
class ReverseT<List, false>
    : public PushBackT<Reverse<PopFront<List>>, Front<List>> {};
// Base case. Identity function on an empty typelist.
template <typename List> class ReverseT<List, true> {
  using Type = List;
};

// Pop from the back of the typelist and return the remaining typelist.
template <typename List> class PopBackT {
public:
  // Reverse the typelist, pop from the front and reverse back to the original
  // order sans the removed element.
  using Type = Reverse<PopFront<Reverse<List>>>;
};
template <typename List> using PopBack = typename PopBackT<List>::Type;

#endif // !CPP_TEMPLATES_CHAPTER24_TYPELISTS
