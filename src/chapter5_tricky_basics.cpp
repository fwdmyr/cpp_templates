#include "../include/chapter5_tricky_basics.hpp"

int main() {
  const auto int_wrapper = ContainerWrapper<int, std::vector>{};
  auto double_wrapper = ContainerWrapper<double, std::deque>{};
  double_wrapper = int_wrapper;

  const auto bs = std::bitset<10U>{420U};
  print_bitset(bs);

  std::cout << pi_approx<float> << '\n';
}
