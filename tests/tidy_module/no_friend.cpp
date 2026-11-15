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

namespace crtp {

// A CRTP base befriends its derived class, its own type parameter, so that
// only that class makes one, as bugprone-crtp-constructor-accessibility asks.
template <typename Self>
class Countable {
 public:
  int count() const { return count_; }

 private:
  Countable() = default;
  friend Self;

  int count_{0};
};

class Apples : public Countable<Apples> {};

// A type parameter of an enclosing class template is no CRTP base's own.
template <typename Outer>
class Holder {
  template <typename Inner>
  class Part {
    friend Outer;  // house: friend
  };
};

// A derived class befriends its own CRTP base, a direct base whose template
// argument is the class itself: the base reaches the hooks it calls, which stay
// private to everyone else.
template <typename Self>
class Scaled {
 public:
  int Twice() const { return 2 * static_cast<Self const&>(*this).Value(); }
};

class Meters : public Scaled<Meters> {
  friend Scaled<Meters>;

  int Value() const { return 3; }
};

// In a class template: through an alias of the base, and by the injected name.
template <typename Unit>
class Length : public Scaled<Length<Unit>> {
  using Base = Scaled<Length<Unit>>;
  friend Base;

  int Value() const { return 1; }
};

template <typename Unit>
class Width : public Scaled<Width<Unit>> {
  friend Scaled<Width>;

  int Value() const { return 1; }
};

int Total() {
  return Meters{}.Twice() + Length<int>{}.Twice() + Width<int>{}.Twice();
}

// A base of another class's, or no base at all, is not the class's own.
class Feet : public Scaled<Meters> {
  friend Scaled<Meters>;  // house: friend
};

class Yards {
  friend Scaled<Yards>;  // house: friend
};

// A base that is no CRTP base.
class Plain {};

class Child : public Plain {
  friend Plain;  // house: friend
};

}  // namespace crtp

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
