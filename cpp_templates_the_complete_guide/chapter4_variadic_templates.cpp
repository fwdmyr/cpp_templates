#include "chapter4_variadic_templates.hpp"

int main() {
  const auto str = "grep";
  variadic_print(5.2, "hello", 69, 1.1, str);
  variadic_print_line(5.2, "hello", 69, 1.1, str);
  std::cout << "Parameter pack size is " << size(5.2, "hello", 69, 1.1, str)
            << '\n';

  auto initial_value = 5.0;
  std::cout << "Folded sum of squares is "
            << fold_square_and_add(std::move(initial_value), 1.0, 2.0, 3.0, 4.0)
            << '\n';

  std::cout << std::boolalpha;
  std::cout << "Folded and is " << fold_and(true, false, true) << '\n';

  std::cout << "Folded homogenity check is "
            << fold_is_homogeneous(5.0, 1.0F, 2U) << '\n';

  auto root = new Node{0};
  root->left = new Node{1};
  root->left->right = new Node{2};
  root->left->right->right = new Node{3};
  const auto target = fold_traverse(root, left, right, right);
  std::cout << "Target node value is " << target->value << '\n';

  const auto str_array =
      std::array<std::string, 5U>{"darkness", "hello", "my", "friend", "old"};
  variadic_print_indices<1U, 0U, 2U, 4U, 3U>(str_array);
  variadic_print_indices(str_array, Index<1U, 0U, 2U, 4U, 3U>{});

  const auto mocked_array = MockedArray{1, 2, 3, 4, 5};

  return 0;
}
