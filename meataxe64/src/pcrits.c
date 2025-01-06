/*
 * Implementations of assembler pcrit routines in C for portability
 */

#include <stdint.h>
#include <stddef.h>
#include "field.h"
#include "pcrit.h"
#include "utils.h"
#include <string.h>
#include <stdio.h>
#include <assert.h>

/* pcpmad: return (A * B + C) mod p */
/* This has to maintain 128 bit precision internally */

/* Reduce function. Reduces a * 2^i mod p */
/* overflow gives 2^64 mod p, which we need internally */
static uint64_t reduce(uint64_t a, uint64_t i, uint64_t p, const FIELD *f)
{
  uint64_t b;
  /* We grease using the reductions table */
  assert(0 == i % 4);
  while (i > 0) {
    a %= p; /* Keep in range 0 <= a < p */
    b = a >> 60; /* Top four bits */
    b = f->reds[b]; /* Look up the reduction */
    a <<= 4;
    a += b; /* Add in overflow */
    if (a < b) {
      /* Overflowed 64 bits */
      a += f->reds[1];
    }
    i -= 4;
  }
  return a % p;
}

#define ff32 0xffffffff
#define ff64 0xffffffffffffffff

/*
 * This is too slow
 * Takes about 10 minutes for something that used to take 2 seconds
 * Ie factor 300 worse
 * After improvements down to around 3 minutes. Factor 100 to go
 * We can grease the reductions by computing, at field set up,
 * i * 2 ^ 64 mod p for 0 <= i < 16
 * This will reduce the loop to 8 cycles rather than 32
 * That gets us another factor 4, leaving 25 to go
 * Example
zrd a1 18446744073709551577 2111 2111
time zmu a1 a1 a2
 */
uint64_t pcpmad(uint64_t p, uint64_t a, uint64_t b, uint64_t c, const FIELD *f)
{
  if (p <= ff32) {
    /* The multiply can't overflow */
    uint64_t res = (a * b) % p; /* Reduce to ensure the add doesn't overflow */
    return (res + c) % p;
  } else {
    uint64_t l_a = a & ff32,
      h_a = a >> 32,
      l_b = b & ff32,
      h_b = b >> 32,
      r1 = (l_a * l_b),
      r2 = (l_a * h_b),
      r3 = (h_a * l_b),
      r4 = (h_a * h_b),
      reducer = (1 + (ff64 % p)) % p;
    /* If the reducer is at most ff32 then we can do
     * another multiply splitting r4 into hi and lo
     * and thus only need one multiply by 2^32
     */
    /* We do r2 and r3 together. They can only overflow 1 bit */
    /* We can also reduce r4 32 bits, and then add into r2+r3 */
    r4 = reduce(r4, 32, p, f); /* Bring into same range as r2, r3 */
    r2 += r3;
    if (r2 < r3) {
      /* We've overflowed, add in 2^64 mod p */
      r2 += reducer;
    }
    r2 += r4;
    if (r2 < r4) {
      /* We've overflowed, add in 2^64 mod p */
      r2 += reducer;
    }
    r2 = reduce(r2, 32, p, f);
    /* Now have all the remainders in 64 bits,
     * and all in the range 0 <= x < p.
     * add up and deal with overflow as we go */
    r1 += r2;
    if (r1 < r2) {
      r1 += reducer;
    }
    r1 += c;
    if (r1 < c) {
      r1 += reducer;
    }
    /* Finally reduce mod p */
    return r1 % p;
  }
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

void pc1xorj(Dfmt *d, const Dfmt *s1, const Dfmt *s2, uint64_t nob)
{
  pc1xora(d, s1, s2, nob);
}

/* Perform a 128 bit by 64 bit divide
 * No catering for divisor bigger than hi
 * where result would overflow 64 bits */
static uint64_t div128_64(uint64_t hi, uint64_t lo, uint64_t divisor)
{
  uint64_t res = 0;
  unsigned int count = 64;
  assert(hi < divisor); /* Programming error if not */
  while (count > 0) {
    res <<= 1; /* Shift before adding */
    hi <<= 1;
    if ((int64_t)lo < 0) {
      /* Top bit set and about to be shifted out */
      hi += 1;
      lo <<= 1;
    }
    count--; /* One less bit to go */
    if (hi >= divisor) {
      res += 1;
      hi -= divisor;
    }
  }
  return res;
}

/* Automated (and currently incorrect) conversion */
void pcbarprp(int inp, int oup, uint64_t base, int digits,
              uint64_t maxval, uint64_t *barpar)
{
    uint64_t tmp;
    barpar[1] = base;
    digits -= 1;                  /* one less than actual digits */
    barpar[2] = digits;
    digits = 0;                   /* number of bits in base */

    while (base != 0) {
      base >>= 1;
      if (base != 0) {
        digits++;
      }
    }
    /* digits is number of bits to shift */
    barpar[3] = digits;
    base = 1;
    base <<= (uint8_t)(digits & 0xff);
    /* We want (base << digits) * 2^64 / barpar[1]
     * We round up the quotient and store in barpar[4]
     * We ignore the remainder
     */
    /* We need a 128 bit by 64 bit divide */
    barpar[4] = 1 + div128_64(base, 0, barpar[1]);
    tmp = 0;                   /* start flags at zero */
    if ((int64_t)maxval < 0) {            /* OK if max to bit not set */
      tmp += 16;             /* else first round is divide */
    }
    if (inp != 1) {
      tmp += 4;
      if (inp != 2) {
        tmp += 4;
      }
    }
    if (oup <= 2) {
      tmp += 1;
      if (oup <= 1) {
        tmp += 1;
      }
    }
    barpar[0] = tmp;               /* flags */
}

void pccl32(const uint64_t *clpm, uint64_t scalar, uint64_t noc, 
            uint32_t *d1, uint32_t *d2)
{
  size_t offs = offsetof(FIELD, clpm);
  FIELD *f = (FIELD *)((uint8_t *)clpm - offs); /* Find the field */
  uint64_t i, x64;
  scalar >>= f->clpm[2]; /* Get the scalar back to the original */
  for(i = 0; i < noc;i++) {
    x64 = qmul(f, *(d1++), scalar) ^ *d2;
    *(d2++) = x64;
  }
}

void pccl64(const uint64_t *clpm, uint64_t scalar, uint64_t noc, 
            uint64_t *d1, uint64_t *d2)
{
  size_t offs = offsetof(FIELD, clpm);
  FIELD *f = (FIELD *)((uint8_t *)clpm - offs); /* Find the field */
  uint64_t i, x64;
  scalar >>= f->clpm[2]; /* Get the scalar back to the original */
  for(i = 0; i < noc;i++) {
    x64 = qmul(f, *(d1++), scalar) ^ *d2;
    *(d2++) = x64;
  }
}
