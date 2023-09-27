#include <iostream>

class Complex {
public:
  Complex() = default;
  explicit Complex(double real, double imag = 0) noexcept
      : m_real{real}, m_imag{imag} {}

  Complex &operator+=(const Complex &other) {
    m_real += other.real();
    m_imag += other.imag();
    return *this;
  }

  Complex &operator++() {
    ++m_real;
    ++m_imag;
    return *this;
  }

  Complex operator++(int) {
    auto tmp = Complex{*this};
    ++tmp;
    return tmp;
  }

  double real() const noexcept { return m_real; }
  double imag() const noexcept { return m_imag; }

private:
  double m_real{};
  double m_imag{};
};

inline Complex operator+(Complex lhs, const Complex &rhs) {
  lhs += rhs;
  return lhs;
}
inline std::ostream &operator<<(std::ostream &os, const Complex &c) {
  os << '(' << c.real() << ',' << c.imag() << ')' << '\n';
  return os;
}
