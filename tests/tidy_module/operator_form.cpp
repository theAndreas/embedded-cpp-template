// The cases of house-operator-form. A line that the check reports ends in a
// comment `house: hidden` for an operator that is to be a hidden friend, or
// `house: member` for one that is to be a member; any other line must not be
// reported. tests/tidy_module.py runs the check on this file and compiles it as
// C++11.

namespace right {

// Every operator in its form: a member where it changes its operand, is unary
// or the language makes it one; a hidden friend where it is binary and leaves
// its operands as they are.
class Duration {
 public:
  explicit Duration(unsigned milliseconds) : milliseconds_{milliseconds} {}
  Duration(Duration const&) = default;
  Duration(Duration&&) = default;
  ~Duration() = default;

  Duration& operator=(Duration const&) = default;
  Duration& operator=(Duration&&) = default;
  Duration& operator+=(Duration other) {
    milliseconds_ += other.milliseconds_;
    return *this;
  }
  Duration& operator++() {
    ++milliseconds_;
    return *this;
  }
  Duration operator++(int) {
    Duration const before{*this};
    ++milliseconds_;
    return before;
  }
  Duration operator-() const { return Duration{0U - milliseconds_}; }
  bool operator!() const { return milliseconds_ == 0U; }
  unsigned operator[](unsigned index) const { return milliseconds_ + index; }
  unsigned operator()() const { return milliseconds_; }
  explicit operator bool() const { return milliseconds_ != 0U; }

  friend Duration operator+(Duration a, Duration b) { return a += b; }
  friend Duration operator-(Duration a, Duration b) {
    return Duration{a.milliseconds_ - b.milliseconds_};
  }
  friend bool operator==(Duration a, Duration b) {
    return a.milliseconds_ == b.milliseconds_;
  }
  friend bool operator<(Duration a, Duration b) {
    return a.milliseconds_ < b.milliseconds_;
  }

 private:
  unsigned milliseconds_;
};

// In a class template the same, as written.
template <typename Value>
class Box {
 public:
  explicit Box(Value value) : value_{value} {}

  Box& operator+=(Box other) {
    value_ += other.value_;
    return *this;
  }
  friend bool operator==(Box a, Box b) { return a.value_ == b.value_; }

 private:
  Value value_;
};

bool Same(Box<int> a, Box<int> b) { return (a += b) == b; }

// An enum can have no friend: its operators are free functions.
enum class Mode : unsigned char { kOff, kOn };
inline Mode operator~(Mode mode) {
  return mode == Mode::kOff ? Mode::kOn : Mode::kOff;
}
inline bool operator<(Mode a, Mode b) {
  return static_cast<unsigned char>(a) < static_cast<unsigned char>(b);
}

}  // namespace right

namespace wrong {

class Fixed {
 public:
  explicit Fixed(int raw) : raw_{raw} {}

  // Binary, leaving its operands: a member, but to be a hidden friend.
  bool operator==(Fixed const& other) const { return raw_ == other.raw_; }  // house: hidden
  bool operator<(Fixed const& other) const;  // house: hidden
  Fixed operator-(Fixed const& other) const { return Fixed{raw_ - other.raw_}; }  // house: hidden

  // Changing its operand, or unary: a friend, but to be a member.
  friend Fixed& operator*=(Fixed& a, Fixed const& b) {  // house: member
    a.raw_ *= b.raw_;
    return a;
  }
  friend Fixed operator-(Fixed const& a) { return Fixed{-a.raw_}; }  // house: member

  int raw() const { return raw_; }

 private:
  int raw_;
};

// Defined outside the class: reported at its declaration, once.
bool Fixed::operator<(Fixed const& other) const { return raw_ < other.raw_; }

// Free functions for the class of this file.
bool operator!=(Fixed const& a, Fixed const& b) { return !(a == b); }  // house: hidden
Fixed operator+(Fixed const& a, int b) { return Fixed{a.raw() + b}; }  // house: hidden
Fixed& operator-=(Fixed& a, Fixed const& b) {  // house: member
  a = a - b;
  return a;
}
bool operator!(Fixed const& a) { return a.raw() == 0; }  // house: member

// Two parameters of a class template, also as written.
template <typename Value>
class Pair {
 public:
  Value first;
};

template <typename Value>
bool operator==(Pair<Value> const& a, Pair<Value> const& b) {  // house: hidden
  return a.first == b.first;
}

bool Same(Pair<int> const& a, Pair<int> const& b) { return a == b; }

}  // namespace wrong
