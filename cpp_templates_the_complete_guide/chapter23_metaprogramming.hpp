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

// Hybrid metaprogramming for unit types in the style of std::chrono..

template <unsigned N, unsigned D = 1> struct Ratio {
  static constexpr unsigned num = N;
  static constexpr unsigned den = D;
  using Type = Ratio<num, den>;
};

// Adds two ratios, i.e. den1/num1 + den2/num2.
template <typename R1, typename R2> class RatioAddImpl {
private:
  static constexpr unsigned den = R1::den * R2::den;
  static constexpr unsigned num = R1::num * R2::den + R2::num * R1::den;

public:
  using Type = Ratio<num, den>;
};
template <typename R1, typename R2> using RatioAdd = RatioAddImpl<R1, R2>;

// Duration type for values of type T with unit type U.
template <typename T, typename U = Ratio<1>> class Duration {
public:
  using ValueType = T;
  using UnitType = typename U::Type;

private:
  ValueType val_;

public:
  constexpr Duration(ValueType val = 0) noexcept : val_{val} {}
  constexpr ValueType value() const noexcept { return val_; }
};

// Adds two durations with arbitrary unit types.
template <typename T1, typename U1, typename T2, typename U2>
constexpr auto operator+(const Duration<T1, U1> &lhs,
                         const Duration<T2, U2> &rhs) {
  // Resulting unit type has numerator of 1 and denominator as a result of
  // adding both fractions.
  using RR = Ratio<1, RatioAdd<U1, U2>::den>;

  // Sum of both values converted to the resulting unit type.
  auto val = lhs.value() * RR::den / U1::den * U1::num +
             rhs.value() * RR::den / U2::den * U2::num;

  return Duration<decltype(val), RR>{val};
}

#endif // !CPP_TEMPLATES_CHAPTER23_METAPROGRAMMING
