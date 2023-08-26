#ifndef CPP_TEMPLATES_CHAPTER5_TRICKY_BASICS
#define CPP_TEMPLATES_CHAPTER5_TRICKY_BASICS

#include <bitset>
#include <deque>
#include <iostream>
#include <string>
#include <vector>

// The template template idiom that allows us to specify the template taking a
// value and container type like this: ContainerWrapper<int, std::vector>.
// The Container template parameter is therefore itself a class template.
// Before C++17, we need to specify all template parameters (including those
// with default values like Alloc here) and need to use class instead of
// typename for the nested class template.
template <typename T,
          template <typename E, typename = std::allocator<E>> class Container>
class ContainerWrapper {
public:
  // Enables copy-assignment for types T_ that can be implicitly converted to T
  // This does not disable the implicit generation of special member functions,
  // so for std::is_same_v<T,T_>==true, the implicitly generated copy-assignment
  // operator would be called.
  // This comes with the usual risks that the special member functions might
  // get called silently when we would expect our declared operator to be called
  // instead leading to unexpected behavior.
  template <typename T_, template <typename E_, typename = std::allocator<E_>>
                         class Container_>
  ContainerWrapper &operator=(const ContainerWrapper<T_, Container_> &rhs);

  // Allow access to private members between all specialized class templates.
  // Since we do not refer to the type and class names, we can omit them here.
  template <typename, template <typename, typename> class>
  friend class ContainerWrapper;

private:
  Container<T> data;
};

template <typename T,
          template <typename E, typename = std::allocator<E>> class Container>
template <typename T_, template <typename E_, typename = std::allocator<E_>>
                       class Container_>
ContainerWrapper<T, Container> &ContainerWrapper<T, Container>::operator=(
    const ContainerWrapper<T_, Container_> &rhs) {
  std::cout << "ContainerWrapper<" << typeid(T).name() << ','
            << typeid(Container<T>).name()
            << ">::operator=(const ContainerWrapper<" << typeid(T_).name()
            << ',' << typeid(Container_<T_>).name() << "> &rhs)" << '\n';
  data.clear();
  // Friend declaration allows us to access rhs.data here
  data.insert(data.begin(), rhs.data.begin(), rhs.data.end());
  return *this;
}

template <std::size_t N> void print_bitset(std::bitset<N> const &bs) {
  // The .template construct is required here as the compiler cannot deduce if
  // "<" corresponds to operator< or the beginning of a template argument list.
  // This issue only appears if the name before the ".template" depends on a
  // template parameter which is the case for std::bitset<N> here.
  std::cout << bs.template to_string<char, std::char_traits<char>,
                                     std::allocator<char>>()
            << '\n';
}

// Variable templates are useful to provide constants of different type T.
template <typename T, typename = std::enable_if_t<std::is_floating_point_v<T>>>
constexpr T pi_approx = T{3.14159265};

#endif // !CPP_TEMPLATES_CHAPTER5_TRICKY_BASICS
