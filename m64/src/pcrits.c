/*
 * Implementations of assembler pcrit routines in C for portability
 * WIP
 */

#include <stdint.h>
#include <stddef.h>
#include "field.h"
#include "pcrit.h"
#include "utils.h"
#include <string.h>
#include <stdio.h>
#include <assert.h>

#define ff32 0xffffffff

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

/* A multiply to deliver the top 64 bits of the result */
static uint64_t top_multiply(uint64_t a, uint64_t b)
{
  /* Split the multiplicands into hi and lo and form the products */
  uint64_t l_a = a & ff32,
    h_a = a >> 32,
    l_b = b & ff32,
    h_b = b >> 32,
    r1 = (l_a * l_b),
    r2 = (l_a * h_b),
    r3 = (h_a * l_b),
    r4 = (h_a * h_b),
    tmp;
  /* Now compute the overflows */
  r4 += (r2 >> 32) + (r3 >> 32); /* Add in the hi parts from the 2^32 results */
  r2 &= ff32;
  r2 += r3 & ff32; /* Get the lo part from the 2^32 */
  tmp = r2 >> 32; /* Get the overflow bit */
  r4 += tmp;
  r2 <<= 32; /* Adjust so we can add in r1 */
  r2 += r1;
  if (r2 < r1) {
    /* Overflowed 64 bits, increment r4 */
    r4 += 1;
  }
  return r4;
}


/* pcpmad: return (A * B + C) mod p */
/* This has to maintain 128 bit precision internally */
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
      reducer = f->reds[1];
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

/* Barrett multiplication
 * (see eg https://en.wikipedia.org/wiki/Barrett_reduction)
 * Initial set up
 */
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
      digits++;
    }
    digits--;
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
  for(i = 0; i < noc; i++) {
    x64 = qmul(f, *(d1++), scalar) ^ *d2;
    *(d2++) = x64;
  }
}

/* This doesn't appear to be a classical Barrett,
 * where we want ab mod p without having to do real divides
 * Rather, it seems to take a value in an extension field
 * expressed as Sigma ai*p^i and deliver the sequence ai
 * The first digit is obtained by a divide, but the others
 * somehow arrive via a multiply by a precomputed constant
 */
void pcbarrett(const uint64_t *params, const Dfmt *input, Dfmt *output,
                uint64_t entries, uint64_t stride)
{
  uint8_t flags = params[0] & 0xff;
  uint64_t base = params[1]; /* base = denominator */
  uint8_t shift = params[3] & 0xff;
  uint8_t digits;
  uint8_t *output_copy; /* output pointer */
  uint64_t dividend;
  uint64_t quotient = 0;
  uint64_t remainder;
  uint64_t i;
  int pass = 1;

  for (i = 0; i < entries; i++) {
    digits = params[2] & 0xff; /* copy of digits */
    output_copy = output;
    if (flags & 0x0C) { /* 32-bit input? */
      /* input is not 32 bits */
      if (flags & 0x04) { /* 64-bit load? */
        dividend = (*(uint16_t *)input) & 0xffff; /* 16 bit load with zero extension */
        input += 2; /* next 16 bit number */
      } else {
        dividend = *(uint64_t *)input; /* 64-bit load */
        input += 8; /* next 64-bit number */
      }
    } else {
      dividend = (*(uint32_t *)input) & ff32; /* 32-bit load with zero extension */
      input += 4; /* next 32-bit number */
    }
    while (digits > 0) {
      if ((flags & 0x10) && pass) { /* do we do first digit by division */
        remainder = dividend % base;
        dividend /= base;
      } else {
        remainder = dividend; /* save a copy of dividend in remainder */
        /* This is the Barrett divide, done by multiply
         * by the precomputed constant followed by a shift.
         * In standard Barrett the quotient may be off by 1 (too small)
         * This code doesn't seem to take account of that, or
         * maybe it has a clever workaround
         */
        quotient = top_multiply(dividend, params[4]);
        quotient >>= shift; /* complete the Barrett */
        dividend = quotient; /* We'll divide this again in the next round */
        quotient *= base; /* Now get the exact multiple to obtain the remainder */
        remainder -= quotient;
      }
      pass = 0;
      if (flags & 0x03) { /* 8 bit output */
        /* store is not 8 bits */
        if (flags & 0x02) { /* 16 bit store? */
          *(uint32_t *)output_copy = (uint32_t)remainder; /* 32-bit store */
        } else {
          *(uint16_t *)output_copy = (uint16_t)remainder; /* 16 bit store */
        }
      } else {
        *output_copy = (uint8_t)remainder; /* 8 bit store */
      }

      output_copy += stride; /* increment output pointer */
      digits--; /* decrement digit count */
    }
    if (flags & 0x03) { /* last digit */
      /* last store, not 8 bits */
      if (flags & 0x02) { /* 16 bit store? */
        *(uint32_t *)output_copy = dividend & ff32; /* 32-bit store */
        output += 4; /* 32-bit increment */
      } else {
        *(uint16_t *)output_copy = (uint16_t)(dividend & 0xffff); /* 16 bit store */
        output += 2; /* 16 bit increment */
      }
    } else {
      *output_copy = (uint8_t)(dividend & 0xFF);
      output += 1;
    }
    continue;
  }
  return;
}

/*
 * Add or subtract for 8 bit fields of characteristic not 2
 * Uses a table of 65536 entries (t2)
 */
/* An auto translated version of pcbif */
void pcbif(Dfmt *rdi, const Dfmt *rsi, const Dfmt *rdx, uint64_t rcx, const uint8_t *table)
{
  uint64_t rbp = 0;
  uint8_t ch, dh;

  /* The original swaps r8 and rsi here. I've renamed instead */
  if (rcx == 0) {
    return;
  }

  if (rcx > 8) {
    uint16_t rbx;
    uint16_t rax;
    uint16_t tmpa;
    uint16_t tmpb;
    rcx -= 4;
    rbx = *(uint16_t *)(rsi + rbp); /* I think this only loads a byte, but we want a word */
    rax = *(uint16_t *)(rdx + rbp);
    /* xchgb ah, bl */
    tmpa = (rax & 0x00ff) | ((rbx & 0xff) << 8);
    tmpb = ((rax & 0xff00) >> 8) | (rbx & 0xff00);
    /*
     * tmpa contains bits 0-7 of a in 0-7 and bits 0-7 of b in 8-15
     * tmpb does the same for bits 8 - 15
     * Thus they can access the 65536 byte table in table
     */
    
    ch = table[tmpa];
    dh = table[tmpb];
    rbx = *(uint16_t *)(rsi+ rbp + 2); /* I think this only loads a byte, but we want a word */
    rax = *(uint16_t *)(rdx + rbp + 2);
    tmpa = (rax & 0x00ff) | ((rbx & 0xff) << 8);
    tmpb = ((rax & 0xff00) >> 8) | (rbx & 0xff00);
    rdi[rbp] = ch;
    ch = table[tmpa];
    rdi[rbp + 1] = dh;
    dh = table[tmpb];
    rbp += 4;
    /* This loop is wrong; there's inline code and then a loop (pcbif4) */
    do {
      /* Check this stuff, it may not be as above */
      rbx = *(uint16_t *)(rsi + rbp); /* I think this only loads a byte, but we want a word */
      rax = *(uint16_t *)(rdx + rbp);
      tmpa = (rax & 0x00ff) | ((rbx & 0xff) << 8);
      tmpb = ((rax & 0xff00) >> 8) | (rbx & 0xff00);
      rdi[rbp - 2] = ch;
      ch = table[tmpa];
      rdi[rbp - 1] = dh;
      dh = table[tmpb];
      rbx = *(uint16_t *)(rsi+ rbp + 2); /* I think this only loads a byte, but we want a word */
      rax = *(uint16_t *)(rdx + rbp + 2);
      tmpa = (rax & 0x00ff) | ((rbx & 0xff) << 8);
      tmpb = ((rax & 0xff00) >> 8) | (rbx & 0xff00);
      rdi[rbp] = ch;
      ch = table[tmpa];
      rdi[rbp + 1] = dh;
      dh = table[tmpb];
      rbp += 4;
    } while (rbp < rcx);
    rdi[rbp - 2] = ch;
    rdi[rbp - 1] = dh;
    rcx += 4;
  }

  do {
    uint8_t r10 = rsi[rbp]; /* Whereas this does want a byte */
    uint8_t r11 = rdx[rbp];
    uint16_t index = (r10 << 8) | r11;
    /* b in bits 8 - 15, a in 0 - 7 */
    rdi[rbp] = table[index];
    rbp++;
  } while (rbp < rcx);
}
