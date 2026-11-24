// The cases of house-copy-move-signature. A line that the check reports ends
// in a comment `house: <how>`; any other line must not be reported.
// tests/tidy_module.py runs the check on this file, applies its fix-its to a
// copy and compiles both as C++11.

namespace reported {

// Without the ref-qualifier, and a move without noexcept: fix-its, as each is
// declared once, where it is defined.
class Plain {
 public:
  Plain() = default;
  Plain(Plain const& other) : value_{other.value_} {}
  Plain(Plain&& other) : value_{other.value_} {}  // house: fix
  Plain& operator=(Plain const& other) {  // house: fix
    value_ = other.value_;
    return *this;
  }
  Plain& operator=(Plain&& other) {  // house: fix
    value_ = other.value_;
    return *this;
  }
  ~Plain() = default;

 private:
  int value_{0};
};

// A move assignment with its ref-qualifier, without noexcept.
class Qualified {
 public:
  Qualified() = default;
  Qualified(Qualified const&) = default;
  Qualified(Qualified&&) noexcept = default;
  Qualified& operator=(Qualified const&) & = default;
  Qualified& operator=(Qualified&& other) & {  // house: fix
    value_ = other.value_;
    return *this;
  }
  ~Qualified() = default;

 private:
  int value_{0};
};

// Declared in the class, defined outside it: no fix-it, as both would have to
// change.
class Split {
 public:
  Split() = default;
  Split(Split const&) = default;
  Split(Split&&) noexcept = default;
  Split& operator=(Split const& other);  // house: qualifier
  Split& operator=(Split&&) & noexcept = default;
  ~Split() = default;

 private:
  int value_{0};
};

Split& Split::operator=(Split const& other) {
  value_ = other.value_;
  return *this;
}

// Defaulted outside the class, which makes it user-provided.
class LateDefault {
 public:
  LateDefault() = default;
  LateDefault(LateDefault const&) = default;
  LateDefault(LateDefault&&) noexcept = default;
  LateDefault& operator=(LateDefault const&);  // house: qualifier
  LateDefault& operator=(LateDefault&&) & noexcept = default;
  ~LateDefault() = default;
};

LateDefault& LateDefault::operator=(LateDefault const&) = default;

// The wrong ref-qualifier, a parameter by value and one that is not const, a
// move that is noexcept(false): no fix-it, as only the author knows the way.
class Odd {
 public:
  Odd() = default;
  Odd(Odd& other) : value_{other.value_} {}  // house: const
  Odd(Odd&& other) noexcept(false) : value_{other.value_} {}  // house: noexcept
  Odd& operator=(Odd other) & {  // house: value
    value_ = other.value_;
    return *this;
  }
  Odd& operator=(Odd&& other) && noexcept {  // house: qualifier
    value_ = other.value_;
    return *this;
  }
  ~Odd() = default;

 private:
  int value_{0};
};

// A move whose parameter is const, which nothing can be moved out of.
class ConstMove {
 public:
  ConstMove() = default;
  ConstMove(ConstMove const&) = default;
  ConstMove(ConstMove const&& other) noexcept  // house: rvalue
      : value_{other.value_} {}
  ConstMove& operator=(ConstMove const&) & = default;
  ConstMove& operator=(ConstMove const&& other) & noexcept {  // house: rvalue
    value_ = other.value_;
    return *this;
  }
  ~ConstMove() = default;

 private:
  int value_{0};
};

// A class template: reported once, at its definition, whatever instantiates
// it.
template <typename T>
class Box {
 public:
  Box() = default;
  Box(Box const&) = default;
  Box(Box&& other) noexcept : value_{other.value_} {}
  Box& operator=(Box const& other) {  // house: fix
    value_ = other.value_;
    return *this;
  }
  Box& operator=(Box&&) & noexcept = default;
  ~Box() = default;

 private:
  T value_{};
};

void UseBoxes() {
  Box<int> a;
  Box<int> b;
  a = b;
  Box<long> c;
  Box<long> d;
  c = d;
}

}  // namespace reported

namespace passed {

// The signatures MISRA C++:2023 15.0.2 gives, written by hand.
class Written {
 public:
  Written() = default;
  Written(Written const& other) : value_{other.value_} {}
  Written(Written&& other) noexcept : value_{other.value_} {}
  Written& operator=(Written const& other) & {
    value_ = other.value_;
    return *this;
  }
  Written& operator=(Written&& other) & noexcept {
    value_ = other.value_;
    return *this;
  }
  ~Written() = default;

 private:
  int value_{0};
};

// Defaulted or deleted where first declared: no user-provided function.
class Defaulted {
 public:
  Defaulted() = default;
  Defaulted(Defaulted const&) = default;
  Defaulted(Defaulted&&) = default;
  Defaulted& operator=(Defaulted const&) = default;
  Defaulted& operator=(Defaulted&&) = default;
  ~Defaulted() = default;
};

class Deleted {
 public:
  Deleted() = default;
  Deleted(Deleted const&) = delete;
  Deleted(Deleted&&) = delete;
  Deleted& operator=(Deleted const&) = delete;
  Deleted& operator=(Deleted&&) = delete;
  ~Deleted() = default;
};

// Implicit ones, which the compiler writes.
struct Implicit {
  int value;
};

// Another assignment, of another type or from a template, is no copy and no
// move.
class Other {
 public:
  Other& operator=(int value) {
    value_ = value;
    return *this;
  }
  template <typename U>
  Other& operator=(U const& value) {
    value_ = static_cast<int>(value);
    return *this;
  }

 private:
  int value_{0};
};

// A noexcept that depends on the template.
template <bool kNothrow>
class Conditional {
 public:
  Conditional() = default;
  Conditional(Conditional const&) = default;
  Conditional(Conditional&& other) noexcept(kNothrow) : value_{other.value_} {}
  Conditional& operator=(Conditional const&) & = default;
  Conditional& operator=(Conditional&& other) & noexcept(kNothrow) {
    value_ = other.value_;
    return *this;
  }
  ~Conditional() = default;

 private:
  int value_{0};
};

void UseConditional() {
  Conditional<true> a;
  Conditional<true> b{static_cast<Conditional<true>&&>(a)};
  a = static_cast<Conditional<true>&&>(b);
}

}  // namespace passed
