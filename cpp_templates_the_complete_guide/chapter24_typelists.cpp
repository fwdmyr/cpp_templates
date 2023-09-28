#include "chapter24_typelists.hpp"

int main() {
  using SignedIntegralTypes = Typelist<short, int, long, long long>;
  // Noop.
  using SignedIntegralTypesAgain =
      PopFront<PushFront<SignedIntegralTypes, bool>>;

  // Create a new typelist by applying transform with add const metafunction.
  using ConstSignedIntegralTypes = Transform<SignedIntegralTypes, AddConstT>;

  // Reverse a list by reducing with PushFrontT as metafunction.
  using ReversedSignedIntegralTypes =
      Reduce<SignedIntegralTypes, PushFrontT, Typelist<>>;

  // Find the largest type in list by reducing with LargerTypeT as metafunction.
  using LargestSignedIntegralType =
      Reduce<PopFront<SignedIntegralTypes>, LargerTypeT,
             Front<SignedIntegralTypes>>;

  return 0;
}
