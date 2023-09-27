#include "chapter24_typelists.hpp"

int main() {
  using SignedIntegralTypes = Typelist<short, int, long, long long>;
  // Noop.
  using SignedIntegralTypesAgain =
      PopFront<PushFront<SignedIntegralTypes, bool>>;


  return 0;
}
