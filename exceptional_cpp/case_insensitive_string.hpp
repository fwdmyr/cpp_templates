#include <cctype>
#include <cstdint>
#include <string>

// My first try

class CIString {
public:
  CIString(const char *str) noexcept : m_str{str} {}
  constexpr auto c_str() const noexcept { return m_str; }

  friend constexpr auto operator==(const CIString &lhs,
                                   const CIString &rhs) noexcept;
  friend constexpr auto operator!=(const CIString &lhs,
                                   const CIString &rhs) noexcept;

private:
  const char *m_str;
};

inline constexpr auto to_lower(char ch) noexcept {
  const auto dec = static_cast<uint8_t>(ch);
  if (dec > 64 && dec < 91)
    return static_cast<char>(ch + 32U);
  return ch;
}

constexpr auto operator==(const CIString &lhs, const CIString &rhs) noexcept {
  auto l_ptr = lhs.c_str();
  auto r_ptr = rhs.c_str();
  while (*l_ptr != '\0' && *r_ptr != '\0') {
    if (!(to_lower(*l_ptr) == to_lower(*r_ptr)))
      return false;
    l_ptr++;
    r_ptr++;
  }
  return *l_ptr == *r_ptr;
}

constexpr auto operator!=(const CIString &lhs, const CIString &rhs) noexcept {
  return !(lhs == rhs);
}

// Solution using char_traits

struct ci_char_traits : public std::char_traits<char> {
  static char to_upper(char ch) { return std::toupper((unsigned char)ch); }

  static bool eq(char c1, char c2) { return to_upper(c1) == to_upper(c2); }

  static bool lt(char c1, char c2) { return to_upper(c1) < to_upper(c2); }

  static int compare(const char *s1, const char *s2, std::size_t n) {
    while (n-- != 0) {
      if (to_upper(*s1) < to_upper(*s2))
        return -1;
      if (to_upper(*s1) > to_upper(*s2))
        return 1;
      ++s1;
      ++s2;
    }
    return 0;
  }

  static const char *find(const char *s, std::size_t n, char a) {
    auto const ua(to_upper(a));
    while (n-- != 0) {
      if (to_upper(*s) == ua)
        return s;
      s++;
    }
    return nullptr;
  }
};
using ci_string = std::basic_string<char, ci_char_traits>;
