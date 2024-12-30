/*
 * Implementations of assembler pcrit routines in C for portability
 */

#include <stdint.h>
#include <stddef.h>
#include "field.h"
#include "pcrit.h"
#include <string.h>
#include <stdio.h>

/* pcpmad: return (A * B + C) mod p */
/* This has to maintain 128 bit precision internally */

/* Reduce function. Reduces a * 2^i mod p */
/* overflow gives 2^64 mod p, which we need internally */
static uint64_t reduce(uint64_t a, uint64_t i, uint64_t p, uint64_t overflow)
{
  uint64_t b;
  while (i > 0) {
    a %= p; /* Reduce. We keep 0 <= a < p */
    b = a;
    a <<= 1; /* Multiply by 2, which may overflow */
    if (a < b) {
      /* we've overflowed */
      a += overflow;
      /* we might overflow again */
      if (a < overflow) {
        a += overflow;
      }
    }
    i--;
  }
  return a % p;
}

#define ff32 0xffffffff
#define ff64 0xffffffffffffffff

uint64_t pcpmad(uint64_t p, uint64_t a, uint64_t b, uint64_t c)
{
  uint64_t l_a = a & ff32,
    h_a = a >> 32,
    l_b = b & ff32,
    h_b = b >> 32,
    r1 = (l_a * l_b) % p,
    r2 = (l_a * h_b) % p,
    r3 = (h_a * l_b) % p,
    r4 = (h_a * h_b) % p,
    reducer = (1 + (ff64 % p)) % p;
  r2 = reduce(r2, 32, p, reducer);
  r3 = reduce(r3, 32, p, reducer);
  r4 = reduce(r4, 64, p, reducer);
  /* Now have all the remainders in 64 bits,
   * and all in the range 0 <= x < p.
   * add up and deal with overflow as we go */
  r1 += r2;
  if (r1 < r2) {
    r1 += reducer;
  }
  r1 += r3;
  if (r1 < r3) {
    r1 += reducer;
  }
  r1 += r4;
  if (r1 < r4) {
    r1 += reducer;
  }
  r1 += c;
  if (r1 < c) {
    r1 += reducer;
  }
  /* Finally reduce mod p */
  return r1 % p;
}

#if 0
void mactype(char *mact)
{
  strcpy(mact, "a      ");
}
#endif

void pc1xora(Dfmt *d, const Dfmt *s1, const Dfmt *s2, uint64_t nob)
{
  while (nob >= 8) {
    *(uint64_t *)d = *(uint64_t *)s1 ^ *(uint64_t *)s2;
    nob -= 8;
    d += 8;
    s1 += 8;
    s2 += 8;
  }
  while (nob > 0 ) {
    *d++ = *s1++ ^ *s2++;
    nob--;
  }
}

/* Automated conversion */
void pcbarprp(int inp, int oup, uint64_t base, int digits,
              uint64_t maxval, uint64_t *barpar)
/*  void pcbarprp(uint64_t rdx, uint64_t rdi, uint64_t rsi, uint64_t r8, uint64_t *r9)*/
{
    uint64_t tmp = 0;
    uint64_t base_rem;
    barpar[1] = base;                // base
    digits -= 1;                  // one less than actual digits
    barpar[2] = digits;                // digits
    digits = 1;                   // number of bits in base

    while (base != 0) {
      base >>= 1;
      if (base != 0) {
        digits++;
      }
    }

    digits -= 1;                  // digits is number of bits to shift
    barpar[3] = digits;
    base = 1;
    base <<= (uint8_t)digits;
    base_rem = base % barpar[1]; // tmp quot, base rem
    tmp = (base_rem == 0) ? 0 : (base_rem + 1); // round up
    barpar[4] = tmp;
    tmp = 0;                   // start flags at zero
    base = maxval >> 63;

    if (base != 0) {            // OK if max to bit not set
        tmp += 16;             // else first round is divide
    }

    if (inp == 1) {
        // do nothing
    } else {
        tmp += 4;
    }

    if (inp == 2) {
        // do nothing
    } else {
        tmp += 4;
    }

    if (oup > 2) {
        // do nothing
    } else {
        tmp += 1;
    }

    if (oup > 1) {
        // do nothing
    } else {
        tmp += 1;
    }

    barpar[0] = tmp;               // flags
}
