#ifndef CPP_TEMPLATES_CHAPTER21_TEMPLATES_AND_INHERITANCE
#define CPP_TEMPLATES_CHAPTER21_TEMPLATES_AND_INHERITANCE

#include <iterator>
#include <type_traits>

// Curiously Recurring Template Pattern (CRTP)
// Pass derived class as a template argument to one of its base classes.

template <typename Derived> class CuriousBase {};
// Looks like this
class Curious : public CuriousBase<Curious> {};
// or this
template <typename T>
class CuriousTemplate : public CuriousBase<CuriousTemplate<T>> {};

// The base class can customize its own behavior to the derived class without
// requiring the use of virtual functions. Therefore, CRTP is useful to factor
// out implementations that can only be member functions (constructor,
// destructor, subscript operators) or are dependent on the derived class's
// identity.

// Example: Count live objects of type.

template <typename T> class ObjectCounter {
private:
  inline static std::size_t count{0U};

protected:
  ObjectCounter() { count++; }
  ObjectCounter(const ObjectCounter<T> &) { count++; }
  ObjectCounter(ObjectCounter<T> &&) { count++; }
  ~ObjectCounter() { count--; }

public:
  static std::size_t live() { return count; }
};

template <typename T> class Countable : public ObjectCounter<Countable<T>> {};
// Since live is public and static, we can query the count like this:
// const auto live_countable_chars = Countable<char>::live();

// Example: Operator implementations

// Factor behavior into base class while retaining the identity of the derived
// classes.
template <typename Derived> struct EqualityComparable {
  // The Barton-Nackman-Trick:
  // Friend function to give both parameters equal behavior for conversions.
  // This also avoids the need for a general operator!= template living in an
  // accessible namespace that matches calls with arguments of any types.
  //
  // NOTE: Friend functions are non-template, non-member functions that are
  // in the scope of the nearest enclosing namespace!!!
  friend bool operator!=(const Derived &lhs, const Derived &rhs) {
    return !(lhs == rhs);
  }
};

struct WithEqualityOperator : public EqualityComparable<WithEqualityOperator> {
  friend bool operator==(const WithEqualityOperator &lhs,
                         const WithEqualityOperator &rhs) {
    // Implement comparison logic
    return bool{};
  }
};

// Example: Iterator facade (p501..508)

// Let CRPT base define the public interface in terms of a much smaller and
// easier to implement interface exposed by the CRTP derived class. This is
// useful when new types need to meet the requirements of an existing interface.

template <typename Derived, typename Value, typename Reference = Value &,
          typename Distance = std::ptrdiff_t>
class ForwardIteratorFacade {
public:
  using value_type = typename std::decay_t<Value>;
  using reference = Reference;
  using pointer = Value *;
  using difference_type = Distance;
  using iterator_category = std::forward_iterator_tag;

  // Implements the general public forward iterator interface in terms of the
  // functions dereference(), increment() and equals() that must be implemented
  // by Derived.
  reference operator*() const { return as_derived().dereference(); }
  Derived &operator++() {
    as_derived().increment();
    return as_derived();
  }
  Derived operator++(int) {
    const auto result = as_derived();
    as_derived().increment();
    return result;
  }
  friend bool operator==(const ForwardIteratorFacade &lhs,
                         const ForwardIteratorFacade &rhs) {
    return lhs.as_derived().equals(rhs.as_derived());
  }

private:
  // Access the derived class.
  Derived &as_derived() { return *static_cast<Derived *>(this); }
  const Derived &as_derived() const {
    return *static_cast<const Derived *>(this);
  }
};

// Implements a node of a singly-linked list.
template <typename T> struct LinkedListNode {
  ~LinkedListNode() { delete next; }
  T value{};
  LinkedListNode<T> *next{nullptr};
};

// Implements the public forward iterator interface for LinkedListNode by CRTP.
template <typename T>
class LinkedListNodeIterator
    : public ForwardIteratorFacade<LinkedListNodeIterator<T>, T> {
private:
  LinkedListNode<T> *current{nullptr};

public:
  LinkedListNodeIterator(LinkedListNode<T> *current = nullptr)
      : current{current} {}

  // Problem: LinkedListNodeIterator exposes public interface!
  // This can be made private by defining an intermediary FacadeAccessor with
  // static member functions that mirror this interface. Both the
  // LinkedListNodeIterator and FacadeAccessor interface are the made private.
  // FacadeAccessor declares ForwardIteratorFacade as a friend,
  // LinkedListNodeIterator declares FacadeAccessor as a friend.
  T &dereference() const { return current->value; }
  void increment() { current = current->next; }
  bool equals(const LinkedListNodeIterator &other) const {
    return current == other.current;
  }
};

// Mixins

// Customize behavior of a type without inheriting from it by inverting the
// usual direction of inheritance. New classes that implement the customization
// are mixed into the inheritance hierarchy as base classes of a class template
// instead of deriving from it. Mixins can be seen as decorators.

template <typename... Mixins> class Point : public Mixins... {
public:
  double x{};
  double y{};
  Point() : Mixins()..., x{}, y{} {}
  Point(double x, double y) : Mixins()..., x{x}, y{y} {}
};

struct Label {
  std::string label{};
};

struct Color {
  uint8_t r{};
  uint8_t g{};
  uint8_t b{};
};

using PointWithLabelAndColor = Point<Label, Color>;
// const auto label = static_cast<Label>(PointWithLabelAndColor{});

// Curious Mixins combine Mixins with CRTP to get even more powerful decorators
// that can tailor their behavior to the type of the derived class. This enables
// the use of template classes like the ObjectCounter (see above) for instance.
template <template <typename> class... Mixins>
class CuriousDerived : public Mixins<CuriousDerived<Mixins...>>... {};

#endif // !CPP_TEMPLATES_CHAPTER21_TEMPLATES_AND_INHERITANCE
