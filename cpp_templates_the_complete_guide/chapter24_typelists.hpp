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

// Transform a typelist.
// Template template parameter MetaFcn maps an input type to an output type.
template <typename List, template <typename T> typename MetaFcn,
          bool Empty = IsEmpty<List>::value>
class TransformT;
// Recursive case. Transform the first element in the typelist and push it to
// the front of the sequence produced by recursively transforming the rest of
// the elements in the typelist.
template <typename List, template <typename T> typename MetaFcn>
class TransformT<List, MetaFcn, false>
    : public PushFrontT<typename TransformT<PopFront<List>, MetaFcn>::Type,
                        typename MetaFcn<Front<List>>::Type> {};
template <typename List, template <typename T> typename MetaFcn>
class TransformT<List, MetaFcn, true> {
public:
  using Type = List;
};
template <typename List, template <typename T> typename MetaFcn>
using Transform = typename TransformT<List, MetaFcn>::Type;

// Metafunction to add const to a type
template <typename T> struct AddConstT {
  using Type = const T;
};
template <typename T> using AddConst = typename AddConstT<T>::Type;

// Reducing a typelist.
// Given typelist T with elements T1, T2, ..., Tn and an initial type I and
// metafunction F. Compute F( ... F(F(I, T1), T2) ... Tn)
template <typename List, template <typename X, typename Y> typename F,
          typename I, bool = IsEmpty<List>::value>
class ReduceT;
// Recursive case. Applies F to the previous result (I) and the front of the
// list. Then passes on the result of applying F as the initial type for the
// reduction of the remainder of the list.
// For each level of recursion, this means that:
// 1) List := PopFront<List>           // Shrink the list to its Tail.
// 2) I    := F<I, Front<List>>::Type  // Accumulate result of F<I, Head>.
template <typename List, template <typename X, typename Y> typename F,
          typename I>
class ReduceT<List, F, I, false>
    : public ReduceT<PopFront<List>, F, typename F<I, Front<List>>::Type> {};
// Base case. Return I that also serves as the accumulator that captures the
// current result. Also ensures that reducing the empty typelist yields the
// initial type.
template <typename List, template <typename X, typename Y> typename F,
          typename I>
class ReduceT<List, F, I, true> {
public:
  using Type = I;
};
template <typename List, template <typename X, typename Y> typename F,
          typename I>
using Reduce = typename ReduceT<List, F, I>::Type;

template <typename T, typename U>
class LargerTypeT : public IfThenElseT<(sizeof(T) >= sizeof(U)), T, U> {};

// Perform insertion sort on a typelist.
// InsertSortedT is a metafunction that inserts a value into a sorted list at
// the first position at which the list would remain sorted.
template <typename List, typename Element,
          template <typename T, typename U> typename Compare,
          bool = IsEmpty<List>::value>
class InsertSortedT;
// Recursive case. Does the following case distinction:
// 1) Prepend the list with the element to be inserted iff it belongs to the
// front of the list.
// 2) Split the list into head and tail, recurse to insert the element into the
// tail and then prepend the head to the result of inserting the element into
// the tail otherwise.
// The recursion in (2) then does another case distinction itself where the new
// list is the previous tail.
template <typename List, typename Element,
          template <typename T, typename U> typename Compare>
class InsertSortedT<List, Element, Compare, false> {
  // Computes the tail of the resulting list using the metafunctions IdentityT
  // and InsertSortedT.
  using NewTail = typename IfThenElse<
      Compare<Element, Front<List>>::value, IdentityT<List>,
      InsertSortedT<PopFront<List>, Element, Compare>>::Type;
  // Computes the head of the resulting list.
  using NewHead =
      IfThenElse<Compare<Element, Front<List>>::value, Element, Front<List>>;

public:
  using Type = PushFront<NewTail, NewHead>;
};
template <typename List, typename Element,
          template <typename T, typename U> typename Compare>
class InsertSortedT<List, Element, Compare, true>
    : public PushFrontT<List, Element> {};
template <typename List, typename Element,
          template <typename T, typename U> typename Compare>
using InsertSorted = typename InsertSortedT<List, Element, Compare>::Type;

template <typename List, template <typename T, typename U> typename Compare,
          bool = IsEmpty<List>::value>
class InsertionSortT;
// Recursive case. Splits the list into head and tail. The tail is sorted
// recursively and the head is inserted into the correct position within the
// inserted list. In other words, inserts the first element into a sorted list.
// Compare takes two types and evaluates to a bool via its value member.
template <typename List, template <typename T, typename U> typename Compare>
using InsertionSort = typename InsertionSortT<List, Compare>::Type;
template <typename List, template <typename T, typename U> typename Compare>
class InsertionSortT<List, Compare, false>
    : public InsertSortedT<InsertionSort<PopFront<List>, Compare>, Front<List>,
                           Compare> {};
template <typename List, template <typename T, typename U> typename Compare>
class InsertionSortT<List, Compare, true> {
public:
  using Type = List;
};

// SmallerThan comparison metafunction for insertion sort.
template <typename T, typename U> class SmallerThanT {
  static constexpr bool value = (sizeof(T) < sizeof(U));
};

#endif // !CPP_TEMPLATES_CHAPTER24_TYPELISTS
