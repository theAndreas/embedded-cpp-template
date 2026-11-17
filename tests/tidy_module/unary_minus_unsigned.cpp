// The cases of house-unary-minus-unsigned. A line that the check reports ends
// in a comment `house: unsigned`; any other line must not be reported.
// tests/tidy_module.py runs the check on this file and compiles it as C++11.

namespace reported {

// An unsigned operand, before its promotion: narrower than int, as wide, and
// a literal.
int Narrow(unsigned char count) {
  return -count;  // house: unsigned
}

unsigned Wide(unsigned count) {
  return -count;  // house: unsigned
}

unsigned Literal() {
  return -1u;  // house: unsigned
}

// In parentheses, the operand is the same.
int Parenthesized(unsigned short count) {
  return -(count);  // house: unsigned
}

// An unscoped enum of an unsigned underlying type ranks as that type.
enum Level : unsigned char { kLow, kHigh };

int Enumerator() {
  return -kHigh;  // house: unsigned
}

// In a template, where an instantiation has an unsigned operand.
template <typename Number>
long Negate(Number value) {
  return -value;  // house: unsigned
}

long Instantiated() { return Negate(1) + Negate(2u); }

}  // namespace reported

namespace passed {

// A signed operand, also one made signed from an unsigned.
int Signed(int value, unsigned char count) {
  int const negated{-value};
  return negated + -static_cast<int>(count);
}

// A character, unsigned as char16_t is: the rule ranks it apart.
int Character(char16_t letter) {
  return -letter;
}

// A floating point operand.
float Floating(float value) { return -value; }

// An overloaded operator-.
class Offset {
 public:
  explicit Offset(unsigned value) : value_{value} {}

  Offset operator-() const { return Offset{value_}; }

 private:
  unsigned value_;
};

Offset Overloaded(Offset offset) { return -offset; }

// A template whose only instantiation is signed.
template <typename Number>
Number Flip(Number value) {
  return -value;
}

int FlippedOnce() { return Flip(3); }

}  // namespace passed
