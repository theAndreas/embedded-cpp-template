// The cases of house-else-if-without-else. A line that the check reports ends
// in a comment `house: chain`; any other line must not be reported.
// tests/tidy_module.py runs the check on this file and compiles it as C++11.

namespace reported {

// The last else if of a chain without else, of two links and of three.
int Two(bool p, bool q) {
  int r{0};
  if (p) {
    r = 1;
  } else if (q) {  // house: chain
    r = 2;
  }
  return r;
}

int Three(bool p, bool q, bool s) {
  int r{0};
  if (p) {
    r = 1;
  } else if (q) {
    r = 2;
  } else if (s) {  // house: chain
    r = 3;
  }
  return r;
}

// A chain in the branch of another if is a chain of its own.
int Nested(bool p, bool q, bool s) {
  int r{0};
  if (p) {
    if (q) {
      r = 1;
    } else if (s) {  // house: chain
      r = 2;
    }
  } else {
    r = 3;
  }
  return r;
}

// In a template: once, as written.
template <typename Number>
Number InTemplate(Number a, bool p, bool q) {
  Number r{a};
  if (p) {
    r = a;
  } else if (q) {  // house: chain
    r = Number{};
  }
  return r;
}

int Instantiated() {
  return InTemplate(1, true, false) +
         static_cast<int>(InTemplate(1L, false, true));
}

}  // namespace reported

namespace passed {

// An if alone, with else or without.
int Alone(bool p) {
  int r{0};
  if (p) {
    r = 1;
  }
  if (p) {
    r = 2;
  } else {
    r = 3;
  }
  return r;
}

// A chain that ends with else, an empty one with its reason as well.
int Ended(bool p, bool q) {
  int r{0};
  if (p) {
    r = 1;
  } else if (q) {
    r = 2;
  } else {
    // Neither: r keeps its start.
  }
  return r;
}

// An if in the then branch of another, without else: not a chain.
int InThen(bool p, bool q) {
  int r{0};
  if (p) {
    if (q) {
      r = 1;
    }
  }
  return r;
}

// In the body of a macro: every expansion shares it.
#define PICK(p, q, r) \
  if (p) {            \
    (r) = 1;          \
  } else if (q) {     \
    (r) = 2;          \
  }

int InBody(bool p, bool q) {
  int r{0};
  PICK(p, q, r)
  return r;
}

}  // namespace passed
