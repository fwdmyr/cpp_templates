#ifndef CPP_TEMPLATES_CHAPTER23_METAPROGRAMMING
#define CPP_TEMPLATES_CHAPTER23_METAPROGRAMMING

#include <cstddef>

// A simple type metafunction that removes all extents of a raw array
// expression.

// Recursive base case.
template <typename T> struct RemoveAllExtentsT {
  using Type = T;
};
// Recursive trait where outermost extent knows its bound.
template <typename T, std::size_t N> struct RemoveAllExtentsT<T[N]> {
  using Type = typename RemoveAllExtentsT<T>::Type;
};
// Recursive trait where outermost extent does not know its bound.
template <typename T> struct RemoveAllExtentsT<T[]> {
  using Type = typename RemoveAllExtentsT<T>::Type;
};
template <typename T>
using RemoveAllExtents = typename RemoveAllExtentsT<T>::Type;

// RemoveAllExtents<int[5][7][]>
// 0) T = int[5][7][]                    // unknown bound specialization
// 1) T = int[5][7]                      // known bound specialization
// 2) T = int[5]                         // known bound specialization
// 3) T = int                            // base case




#endif // !CPP_TEMPLATES_CHAPTER23_METAPROGRAMMING
