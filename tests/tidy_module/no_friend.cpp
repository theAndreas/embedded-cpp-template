// The cases of house-no-friend. A line that the check reports ends in a
// comment `house: friend`; any other line must not be reported.
// tests/tidy_module.py runs the check on this file and compiles it as C++11.

namespace hidden {

// An operator defined in the class, a hidden friend: the one friend allowed.
class Point {
 public:
  explicit Point(int x) : x_{x} {}

  friend bool operator==(Point const& a, Point const& b) { return a.x_ == b.x_; }

  template <typename Other>
  friend bool operator<(Point const& a, Other const& b) {
    return a.x_ < b.x();
  }

 private:
  int x_;
};

// In a class template: as written, and not again in an instantiation.
template <typename Value>
class Box {
 public:
  explicit Box(Value value) : value_{value} {}

  friend bool operator==(Box const& a, Box const& b) {
    return a.value_ == b.value_;
  }

  friend class Point;  // house: friend

 private:
  Value value_;
};

bool Same(Box<int> const& a, Box<int> const& b) { return a == b; }
bool Same(Box<long> const& a, Box<long> const& b) { return a == b; }

}  // namespace hidden

namespace reported {

class Helper;

class Counter {
 public:
  friend class Helper;  // house: friend
  friend void Inspect(int value);  // house: friend
  friend void Reset(Counter& counter) { counter.count_ = 0; }  // house: friend
  // An operator declared here and defined outside the class.
  friend bool operator==(Counter const& a, Counter const& b);  // house: friend

  template <typename Other>
  friend void Copy(Counter& counter, Other const& other);  // house: friend

 private:
  int count_{0};
};

bool operator==(Counter const& a, Counter const& b) {
  return a.count_ == b.count_;
}

}  // namespace reported
