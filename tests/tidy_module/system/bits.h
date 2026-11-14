// A system header to the cases, as avr-libc's <avr/io.h> is to a project: the
// macros of its bits are the platform's names, which house-bitwise-in-bit-class
// does not report.

#ifndef BITS_H_
#define BITS_H_

#define _BV(bit) (1 << (bit))
#define DDB5 5

#endif  // BITS_H_
