// A system header to the cases, as a library's is to a test: a class of
// another file, which an operator of the cases cannot be a friend of, so that
// house-operator-form leaves it a free function.

#ifndef PRINTER_H_
#define PRINTER_H_

namespace library {

struct Printer {
  int width;
};

}  // namespace library

#endif  // PRINTER_H_
