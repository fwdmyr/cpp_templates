#ifndef CPP_TEMPLATES_CHAPTER22_BRIDGING_STATIC_AND_DYNAMIC_POLYMORPHISM
#define CPP_TEMPLATES_CHAPTER22_BRIDGING_STATIC_AND_DYNAMIC_POLYMORPHISM

#include <exception>
#include <memory>
#include <optional>
#include <utility>

// In this chapter, we develop our own function wrapper (like std::function)
// that lambdas, functor structs and normal function pointers can bind to.
// This combines favorable properties of static and dynamic polymorphism.

template <typename R, typename... Args> class FunctorBridge {
public:
  virtual ~FunctorBridge() = default;
  virtual FunctorBridge *clone() const = 0;
  // Making this const protects against invoking non-const operator() overloads.
  virtual R invoke(Args... args) const = 0;
};

// This class bridges between static and dynamic polymorphism. When we define
// specializations for specific functors (i.e. static polymorphism) and create a
// new instance of this derived type on the heap and then assign the resulting
// pointer-to-derived to a pointer-to-base, the type information is lost due to
// the conversion that takes place. The functor whose type is lost can still be
// invoked without knowing its type by using dynamic dispatch via virtual
// functions (i.e. dynamic polymorphism).
template <typename Functor, typename R, typename... Args>
class SpecificFunctorBridge : public FunctorBridge<R, Args...> {
private:
  Functor functor;

public:
  // Functor is of type FunctorType
  template <typename FunctorType>
  SpecificFunctorBridge(FunctorType &&f)
      : functor{std::forward<FunctorType>(f)} {}
  SpecificFunctorBridge *clone() const override {
    return new SpecificFunctorBridge(functor);
  }
  R invoke(Args... args) const override {
    return functor(std::forward<Args>(args)...);
  }
};

// Primary template
template <typename Signature> class FunctionPtr {};

// Partial specialization
template <typename R, typename... Args> class FunctionPtr<R(Args...)> {
private:
  FunctorBridge<R, Args...> *bridge{nullptr};

public:
  // Constructors
  FunctionPtr() = default;
  FunctionPtr(const FunctionPtr &other) { bridge = other.bridge->clone(); };
  FunctionPtr(FunctionPtr &other)
      : FunctionPtr{static_cast<const FunctionPtr &>(other)} {}
  FunctionPtr(FunctionPtr &&other) : bridge{other.bridge} {
    other.bridge = nullptr;
  }
  template <typename FunctionType>
  FunctionPtr(FunctionType &&f) : bridge{nullptr} {

    // The actual function type is only known to the specialization of
    // SpecificFunctorBridge. After the new object of specialized type is
    // created on the heap, the pointer-to-derived converts to pointer-to-base
    // (because bridge is of abstract base class type). When the function
    // returns, the function type is therefore lost.
    // This technique of bridging between static and dynamic polymorphism is
    // therefore called type erasure.
    using Bridge =
        SpecificFunctorBridge<std::decay_t<FunctionType>, R, Args...>;
    bridge = new Bridge(std::forward<FunctionType>(f));
  }
  FunctionPtr &operator=(const FunctionPtr &other) {
    auto tmp{other};
    swap(*this, other);
    return *this;
  }
  // Assignment operators
  FunctionPtr &operator=(FunctionPtr &&other) {
    delete bridge;
    bridge = other.bridge;
    other.bridge = nullptr;
    return *this;
  }
  template <typename FunctionType> FunctionPtr &operator=(FunctionType &&f) {
    FunctionPtr tmp{std::forward<FunctionType>(f)};
    swap(*this, tmp);
    return *this;
  }
  // Destructor
  ~FunctionPtr() { delete bridge; }

  // Helpers
  friend void swap(FunctionPtr &lhs, FunctionPtr &rhs) {
    std::swap(lhs.bridge, rhs.bridge);
  }
  explicit operator bool() const { return bridge == nullptr; }

  // Invocation
  R operator()(Args... args) const {
    return bridge->invoke(std::forward<Args>(args)...);
  }
};

// Another example: Any

struct BadTypeException : std::exception {};

class Any {
private:
  struct HolderBase {
    virtual ~HolderBase() = default;
  };

  template <typename T> struct Holder : public HolderBase {
    Holder(const T &object) : object{object} {}
    Holder(T &&object) : object{std::move(object)} {}
    T object;
  };
  HolderBase *held{nullptr};

public:
  template <typename T>
  Any(T &&object) : held{std::make_unique<T>(std::forward<T>(object))} {}

  bool has_value() const { return !(held == nullptr); }
  template <typename T> const T &get_value() const {
    if (const auto specific_held = dynamic_cast<const Holder<T> *>(held))
      return *specific_held;
    else
      throw BadTypeException{};
  }
};

#endif // !CPP_TEMPLATES_CHAPTER22_BRIDGING_STATIC_AND_DYNAMIC_POLYMORPHISM
