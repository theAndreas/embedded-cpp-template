// The cases of house-braced-member-init. A line that the check reports ends in
// a comment `house: fix`, `house: narrowing` or `house: depends`, which says
// how; any other line must not be reported. tests/tidy_module.py runs the check
// on this file, applies the fix-its to a copy and compiles it as C++11: a
// fix-it that does not compile, or that leaves a line with a fix-it reported,
// fails.

#include <initializer_list>

namespace scalars {

struct Same {
  explicit Same(int value) : value_(value) {} // house: fix
  int value_;
};

struct Widening {
  explicit Widening(int a, float b, unsigned char c, bool d)
      : a_(a),   // house: fix
        b_(b),   // house: fix
        c_(c),   // house: fix
        d_(d) {} // house: fix
  long a_;
  double b_;
  unsigned c_;
  int d_;
};

struct Narrowing {
  explicit Narrowing(long a, double b, int c, int const *p, unsigned u)
      : a_(a),   // house: narrowing
        b_(b),   // house: narrowing
        c_(c),   // house: narrowing
        p_(p),   // house: narrowing
        u_(u) {} // house: narrowing
  int a_;
  float b_;
  double c_;
  bool p_;
  int u_;
};

struct Constants {
  Constants()
      : a_(42L), // house: fix
        b_(1.5), // house: fix
        c_(255), // house: fix
        d_(300), // house: narrowing
        e_(1),   // house: fix
        f_(-1),  // house: narrowing
        g_(3) {} // house: fix
  int a_;
  float b_;
  unsigned char c_;
  unsigned char d_;
  bool e_;
  unsigned f_;
  double g_;
};

struct References {
  explicit References(int &value) : value_(value) {} // house: fix
  int &value_;
};

struct Empty {
  Empty() : value_() {} // house: fix
  int value_;
};

struct AlreadyBraced {
  explicit AlreadyBraced(int value) : value_{value} {}
  int value_;
};

} // namespace scalars

namespace classes {

struct Point {
  Point(int x, int y) : x_{x}, y_{y} {}
  int x_;
  int y_;
};

struct Byte {
  explicit Byte(char value) : value_{value} {}
  char value_;
};

struct List {
  List() = default;
  List(std::initializer_list<int> values) : size_{values.size()} {}
  explicit List(unsigned size) : size_{size} {}
  unsigned long size_{};
};

struct Holder {
  Holder(int x, int value, Holder const &other)
      : point_(x, x),             // house: fix
        byte_(value),             // house: narrowing
        list_(3U), empty_list_(), // house: fix
        copy_(other.point_) {}    // house: fix
  Point point_;
  Byte byte_;
  List list_;
  List empty_list_;
  Point copy_;
};

struct Base {
  explicit Base(int value) : value_{value} {}
  int value_;
};

struct Derived : Base {
  Derived() : Derived(0) {}                    // house: fix
  explicit Derived(int value) : Base(value) {} // house: fix
};

} // namespace classes

namespace templates {

template <typename T> struct Box {
  explicit Box(T value) : value_(value) {} // house: depends
  T value_;
};

template <typename T> struct Count {
  explicit Count(T value) : count_(value) {} // house: fix
  long count_;
};

// Instantiated twice, reported once, where it is written.
Count<int> const kCountInt{1};
Count<short> const kCountShort{1};

struct Converting {
  template <typename U>
  explicit Converting(U value) : value_(value) {} // house: fix
  int value_;
};

template <typename... Bases> struct Mixin : Bases... {
  Mixin() : Bases()... {} // house: depends
};

} // namespace templates

namespace macros {

#define HOUSE_INIT(member, value) member(value)

struct InMacro {
  explicit InMacro(int value) : HOUSE_INIT(value_, value) {}
  int value_;
};

} // namespace macros
