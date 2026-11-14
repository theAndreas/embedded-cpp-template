// The cases of house-bitwise-in-bit-class. A line that the check reports ends
// in a comment `house: bitwise`; any other line must not be reported.
// tests/tidy_module.py runs the check on this file and compiles it as C++11.

#include <bits.h>
#include <stdint.h>

namespace bit_class {

// The bit class itself: inline, in a nested class, and in a member defined
// outside it.
template <typename Integer>
class Bit {
 public:
  class Reference {
   public:
    static Integer Invert(Integer value) { return static_cast<Integer>(~value); }
  };

  static constexpr Integer SetBit(Integer value, uint8_t position) {
    return static_cast<Integer>(value | (1U << position));
  }
  static Integer ClearBit(Integer value, uint8_t position);
};

template <typename Integer>
Integer Bit<Integer>::ClearBit(Integer value, uint8_t position) {
  return static_cast<Integer>(value & Reference::Invert(1U << position));
}

// Instantiated, so that the check sees the instantiations as well.
uint8_t Use(uint8_t value) {
  return Bit<uint8_t>::ClearBit(Bit<uint8_t>::SetBit(value, 1U), 2U);
}

}  // namespace bit_class

namespace masks {

constexpr uint8_t kReady{0x04U};
constexpr uint8_t kError{0x08U};
uint8_t const kIdle{0x40U};
enum Flag : uint8_t { kHot = 0x10U, kLow = 0x20U };

// Named masks or'ed in a constexpr: constants, enumerators, the platform's
// macros, in parentheses and casts.
constexpr uint8_t kBoth{kReady | kError};
constexpr uint8_t kThree{static_cast<uint8_t>(kReady | (kError | kHot))};
constexpr uint8_t kWithConst{kIdle | kLow};
constexpr uint8_t kWithMacro{_BV(DDB5) | kReady};
constexpr int kMacro{_BV(DDB5)};

// Not the exception.
uint8_t const kNotConstexpr{kReady | kError};  // house: bitwise
constexpr uint8_t kLiteral{kReady | 0x01U};  // house: bitwise
constexpr uint8_t kAnd{kReady & kError};  // house: bitwise
constexpr uint8_t kShift{1U << 3U};  // house: bitwise
constexpr uint8_t kOrOfAnd{kReady | (kError & kHot)};  // house: bitwise

// A macro of the project's own is no name of the platform.
#define MASK_OF(bit) (1U << (bit))
constexpr unsigned kOwnMacro{MASK_OF(3U)};  // house: bitwise

}  // namespace masks

namespace code {

uint8_t Read(uint8_t value) {
  uint8_t const masked{static_cast<uint8_t>(value & masks::kReady)};  // house: bitwise
  uint8_t const flipped{static_cast<uint8_t>(~value)};  // house: bitwise
  uint8_t const shifted{static_cast<uint8_t>(value >> 2U)};  // house: bitwise
  uint8_t const named{static_cast<uint8_t>(masks::kReady | masks::kError)};  // house: bitwise
  uint8_t const toggled{static_cast<uint8_t>(value ^ masks::kError)};  // house: bitwise
  return static_cast<uint8_t>(masked + flipped + shifted + named + toggled);
}

void Write(uint8_t volatile& target) {
  target |= masks::kReady;  // house: bitwise
  target &= masks::kError;  // house: bitwise
  target <<= 1U;  // house: bitwise
}

bool Both(bool a, bool b) {
  return a & b;  // house: bitwise
}

// In a template outside the class: reported in the instantiation.
template <typename Integer>
Integer Mask(Integer value, Integer mask) {
  return static_cast<Integer>(value & mask);  // house: bitwise
}

uint8_t UseMask(uint8_t value) { return Mask<uint8_t>(value, masks::kReady); }

}  // namespace code

namespace own_type {

// A type of raw data says what its bits mean: its bitwise operators and the
// builtin ones in their bodies are not reported, nor where it is used.
enum class Byte : unsigned char {};

constexpr Byte operator|(Byte a, Byte b) {
  return static_cast<Byte>(static_cast<unsigned char>(a) |
                           static_cast<unsigned char>(b));
}

inline Byte& operator|=(Byte& a, Byte b) {
  a = a | b;
  return a;
}

// Another operator is no such type's.
inline bool operator==(Byte a, unsigned char b) {
  return (static_cast<unsigned char>(a) & b) != 0U;  // house: bitwise
}

// A shift of the type by a count that gives the type back, and `~` of it:
// its own as well.
constexpr Byte operator<<(Byte a, int count) {
  return static_cast<Byte>(static_cast<unsigned char>(a) << count);
}

constexpr Byte operator~(Byte a) {
  return static_cast<Byte>(~static_cast<unsigned char>(a));
}

Byte Merge(Byte a, Byte b) {
  a |= b;
  return ~(a | b) << 1;
}

// A stream's `<<`, which gives its stream back, is no such type's: an integer
// shifted in its body is reported as anywhere.
struct Log {
  unsigned total;

  friend Log& operator<<(Log& log, Byte byte) {
    log.total = static_cast<unsigned char>(byte) >> 4;  // house: bitwise
    return log;
  }
};

}  // namespace own_type
