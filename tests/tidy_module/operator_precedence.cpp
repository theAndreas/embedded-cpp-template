// The cases of house-operator-precedence. A line that the check reports ends in
// a comment `house: <how>`; any other line must not be reported.
// tests/tidy_module.py runs the check on this file and compiles it as C++11.

namespace reported {

// A comparison as an operand of a logical operator, either side.
bool Logical(int a, int b, bool p, bool q) {
  bool r{a == b && p};  // house: fix
  r = p && a < b;       // house: fix
  r = p || q && r;      // house: fix
  return r;
}

// Arithmetic of a higher rank than the operator.
int Arithmetic(int a, int b, int c) {
  int r{a + b * c};  // house: fix
  r = a - b % c;     // house: fix
  return r;
}

// The condition and the branches of ?: are its operands.
int Conditional(int a, int b) {
  return a == b ? a : a - b;  // house: fix
}

// Nested: each operand of a higher rank, the outer and the inner.
bool Nested(int a, int b, int c, int d) {
  return a + b * c < d;  // house: fix
}

// An overloaded operator ranks as the built-in one it spells: bits of a class
// of their own, and a shift into a stream.
class Mask {
 public:
  explicit Mask(unsigned value) : value_{value} {}

  friend Mask operator&(Mask a, Mask b) { return Mask{a.value_ & b.value_}; }
  friend Mask operator|(Mask a, Mask b) { return Mask{a.value_ | b.value_}; }
  friend Mask operator^(Mask a, Mask b) { return Mask{a.value_ ^ b.value_}; }

 private:
  unsigned value_;
};

Mask Bits(Mask a, Mask b, Mask c) {
  Mask r{a & b | c};  // house: fix
  r = a | b ^ c;      // house: fix
  return r;
}

class Stream {
 public:
  friend Stream& operator<<(Stream& stream, int) { return stream; }
};

Stream& Print(Stream& stream, int a, int b) {
  return stream << a + b;  // house: fix
}

// The operand of sizeof.
unsigned long Size(int a) {
  return sizeof a;  // house: fix
}

// In the argument of a macro, which is written here.
#define ASSERT_THAT(condition) static_cast<void>(condition)

void InArgument(int a, int b, bool p) {
  ASSERT_THAT(a == b && p);  // house: fix
}

// In a template: once, as written.
template <typename Number>
bool InTemplate(Number a, Number b, bool p) {
  return a < b && p;  // house: fix
}

bool Instantiated() {
  return InTemplate(1, 2, true) && InTemplate(1L, 2L, false);
}

}  // namespace reported

namespace passed {

struct Value {
  int x;
};

int Member(Value const& value, int Value::*member) {
  return value.*member + 1;
}

// In parentheses, of the same rank, or of the highest: unary, cast, call.
bool Explicit(int a, int b, int c, bool p, bool q) {
  bool r{(a == b) && p};
  r = (p && (a < b)) || q;
  int s{a + b + c};
  s = a * b / c;
  r = !p && q;
  s = static_cast<int>(p) + a;
  s = -a + b;
  s = p ? a : b;
  s = (a == b) ? a : (a - b);
  return r && (s > 0) && p && q;
}

// The operands of an assignment, and the arguments of a call.
int Assigned(int a, int b, int c) {
  int r{a};
  r = a + b;
  r += a * b;
  r = Assigned(a + b, c, c);
  return r;
}

// The parenthesized operand of sizeof, and sizeof of a type.
unsigned long Sizes(int a) {
  return sizeof(a) + sizeof(int);
}

// In the body of a macro: every expansion shares it.
#define BOTH(x, y) ((x) == 0 && (y) == 0)

bool InBody(int a, int b) {
  return BOTH(a, b);
}

}  // namespace passed
