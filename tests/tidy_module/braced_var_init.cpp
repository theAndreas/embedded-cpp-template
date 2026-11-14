// The cases of house-braced-var-init. A line that the check reports ends in a
// comment `house: fix`, `house: narrowing`, `house: depends` or
// `house: explicit`, which says how; any other line must not be reported.
// tests/tidy_module.py runs the check on this file, applies the fix-its to a
// copy and compiles it as C++11: a fix-it that does not compile, or that leaves
// a line with a fix-it reported, fails.

#include <initializer_list>

namespace scalars {

int const kGlobal = 1;      // house: fix
static int s_static(2);     // house: fix
int const kAlready{3};
char const kName[] = "font";
int s_uninitialized;

int Locals(int value, double ratio, long wide, int const *pointer) {
  int same = value;         // house: fix
  long widened = value;     // house: fix
  int parenthesized(value); // house: fix
  int listed = {value};     // house: fix
  int nested((value));      // house: fix
  float constant = 1.5;     // house: fix
  unsigned char fits = 255; // house: fix
  unsigned char big = 300;  // house: narrowing
  int truncated = ratio;    // house: narrowing
  int narrowed = wide;      // house: narrowing
  bool is_set = pointer;    // house: narrowing
  int const &reference = same; // house: fix
  char text[] = "abc";
  char listed_text[] = {"abc"}; // house: fix
  char braced_text[]{"abc"};  // house: fix
  int numbers[2] = {1, 2};  // house: fix
  static int calls = 0;     // house: fix
  int braced{value};
  for (int i = 0; i < value; ++i) { // house: fix
    same += i;
  }
  if (int const half = value / 2) { // house: fix
    same += half;
  }
  return same + static_cast<int>(widened) + parenthesized + listed + nested +
         static_cast<int>(constant) + fits + big + truncated + narrowed +
         is_set + reference + text[0] + listed_text[0] + braced_text[0] + numbers[0] + calls +
         braced;
}

int Defaults(int value = 0) { return value; }

int RangeFor(int const (&values)[2]) {
  int sum{0};
  for (int const value : values) {
    sum += value;
  }
  return sum;
}

void Lambdas() {
  auto add = [](int a, int b) { return a + b; }; // house: fix
  add(1, 2);
}

void Catches() {
  try {
  } catch (int error) {
  }
}

} // namespace scalars

namespace automatic {

int Deduced(int value, short narrow) {
  auto copy = value;                       // house: fix
  auto cast = static_cast<long>(narrow);   // house: fix
  auto const *pointer = &value;            // house: fix
  auto &reference = value;                 // house: fix
  auto parenthesized(value);               // house: fix
  auto list = {1, 2};
  auto single = {1};
  return copy + static_cast<int>(cast) + *pointer + reference + parenthesized +
         static_cast<int>(list.size() + single.size());
}

} // namespace automatic

namespace classes {

struct Point {
  Point(int x, int y) : x_{x}, y_{y} {}
  int x_;
  int y_;
};

struct Converting {
  Converting(int value) : value_{value} {}
  Converting() : value_{0} {}
  int value_;
};

struct Explicit {
  explicit Explicit(int value) : value_{value} {}
  Explicit(long value) : value_{static_cast<int>(value)} {}
  int value_;
};

struct List {
  List() = default;
  List(std::initializer_list<int> values) : size_{values.size()} {}
  explicit List(unsigned size) : size_{size} {}
  unsigned long size_{};
};

struct Aggregate {
  int a;
  int b;
};

Point MakePoint() { return Point{1, 2}; }

void Objects(Point const &other) {
  Point direct(1, 2);                   // house: fix
  Point copied = other;                 // house: fix
  Point listed = {1, 2};                // house: fix
  Point temporary = Point(3, 4);        // house: fix
  Point braced_temporary = Point{3, 4}; // house: fix
  Point returned = MakePoint();         // house: fix
  Converting converted = 5;             // house: fix
  Converting single = Converting(5);    // house: fix
  Converting empty = Converting();      // house: fix
  Converting narrowing = Converting(1.5); // house: narrowing
  Explicit chosen = 1;                  // house: explicit
  Explicit made = Explicit(1);          // house: fix
  List sized(3U);
  List from_list = {1, 2};              // house: fix
  List from_temporary = List(3U);
  Aggregate aggregate = {1, 2};         // house: fix
  Point already{1, 2};
  (void)direct;
  (void)copied;
  (void)listed;
  (void)temporary;
  (void)braced_temporary;
  (void)returned;
  (void)converted;
  (void)single;
  (void)empty;
  (void)narrowing;
  (void)chosen;
  (void)made;
  (void)sized;
  (void)from_list;
  (void)from_temporary;
  (void)aggregate;
  (void)already;
}

} // namespace classes

namespace members {

struct Defaults {
  int count = 0;                // house: fix
  long widened = 1;             // house: fix
  unsigned char big = 300;      // house: narrowing
  int listed = {2};             // house: fix
  classes::Point point = classes::Point(1, 2); // house: fix
  int braced{3};
  char name[8] = "abc";
  char listed_name[8] = {"abc"};  // house: fix
  char braced_name[8]{"abc"};   // house: fix
  static int const kStatic = 4; // house: fix
  static int shared;
};

int Defaults::shared = 5; // house: fix

} // namespace members

namespace templates {

template <typename T> T Copy(T value) {
  T copy = value;  // house: depends
  T other(value);  // house: depends
  auto deduced = value; // house: fix
  int size = sizeof(T); // house: fix
  return copy + other + deduced + static_cast<T>(size);
}

// Instantiated twice, reported once, where it is written.
int const kInt = Copy(1);   // house: fix
long const kLong = Copy(2L); // house: fix

template <typename T> struct Box {
  T value = T(); // house: depends
  int count = 0; // house: fix
};

Box<int> const kBox{};

} // namespace templates

namespace macros {

#define HOUSE_DECLARE(name, value) int name = value
#define HOUSE_VALUE 7

HOUSE_DECLARE(in_macro, 1);
int const kFromMacro = HOUSE_VALUE; // house: fix

} // namespace macros
