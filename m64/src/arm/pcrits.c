/*
 * Implementations of assembler pcrit routines in C for portability
 * WIP
 */

#define NEON 1

#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include "field.h"
#include "pcrit.h"
#include "utils.h"
#include <string.h>
#include <stdio.h>
#include <assert.h>
#if NEON
#include "arm_neon.h"
#endif

/* Note this assumes little endian implementation */
typedef union union_128 {
  uint64_t longs[2];
  uint32_t ints[4];
} union_128;

/* Overlay a 64 bit integer as 4 16 bit integers */
typedef union union_64_16 {
  uint64_t a;
  uint16_t words[4];
} union_64_16;

/* Overlay a 64 bit integer as 2 32 bit integers */
typedef union union_64_32 {
  uint64_t a;
  uint32_t doubles[2];
} union_64_32;

#define ff32 0xffffffff

/* Functions from pc1.s */

void mactype(char *mact)
{
  mact[0] = 'a'; /* Minimal class */
  mact[1] = '0'; /* Cache class 0 */
}

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

/*
 * A full 128 bit precision 64 x 64 multiply
 * Intel chips have a singled instruction to do this
 * Others don't (necessarily)
 * Operands in a, b
 * Result in lo and function result
 */
static uint64_t full_multiply(uint64_t *lo, uint64_t a, uint64_t b)
{
  /* Split into 32 x 32 which can't overflow */
  uint64_t l_a = a & ff32,
    h_a = a >> 32,
    l_b = b & ff32,
    h_b = b >> 32,
    r1 = (l_a * l_b), /* Bottom 64 bits */
    r2 = (l_a * h_b),
    r3 = (h_a * l_b), /* Bits 32 - 95 */
    r4 = (h_a * h_b), /* Bits 64 - 127 */
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
  *lo = r2;
  return r4;
} 

/* A multiply to deliver the top 64 bits of the result */
static uint64_t top_multiply(uint64_t a, uint64_t b)
{
  uint64_t lo;
  return full_multiply(&lo, a, b);
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

void pc1xora(Dfmt *d, const Dfmt *s1, const Dfmt *s2, uint64_t nob)
{
#if NEON
  while (nob >= 16) {
    *(uint16x8_t *)d = veorq_u16(*(uint16x8_t *)s1, *(uint16x8_t *)s2);
    nob -= 16;
    d += 16;
    s1 += 16;
    s2 += 16;
  }
#else
  while (nob >= 8) {
    *(uint64_t *)d = *(uint64_t *)s1 ^ *(uint64_t *)s2;
    nob -= 8;
    d += 8;
    s1 += 8;
    s2 += 8;
  }
#endif
  while (nob > 0) {
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
void pcbif(Dfmt *dest, const Dfmt *s1, const Dfmt *s2, uint64_t nob, const uint8_t *table)
{
  uint64_t byte_index = 0;
  uint8_t tmp1, tmp2;

  if (nob == 0) {
    return;
  }

  if (nob > 8) {
    uint16_t s1_word;
    uint16_t s2_word;
    uint16_t tmpa;
    uint16_t tmpb;
    nob -= 4;
    s1_word = *(uint16_t *)(s1 + byte_index);
    s2_word = *(uint16_t *)(s2 + byte_index);
    /* xchgb ah, bl */
    tmpa = (s2_word & 0x00ff) | ((s1_word & 0xff) << 8);
    tmpb = ((s2_word & 0xff00) >> 8) | (s1_word & 0xff00);
    /*
     * tmpa contains bits 0-7 of a in 0-7 and bits 0-7 of b in 8-15
     * tmpb does the same for bits 8 - 15
     * Thus they can access the 65536 byte table
     */

    tmp1 = table[tmpa];
    tmp2 = table[tmpb];
    s1_word = *(uint16_t *)(s1+ byte_index + 2);
    s2_word = *(uint16_t *)(s2 + byte_index + 2);
    tmpa = (s2_word & 0x00ff) | ((s1_word & 0xff) << 8);
    tmpb = ((s2_word & 0xff00) >> 8) | (s1_word & 0xff00);
    dest[byte_index] = tmp1;
    tmp1 = table[tmpa];
    dest[byte_index + 1] = tmp2;
    tmp2 = table[tmpb];
    byte_index += 4;
    do {
      s1_word = *(uint16_t *)(s1 + byte_index);
      s2_word = *(uint16_t *)(s2 + byte_index);
      tmpa = (s2_word & 0x00ff) | ((s1_word & 0xff) << 8);
      tmpb = ((s2_word & 0xff00) >> 8) | (s1_word & 0xff00);
      dest[byte_index - 2] = tmp1;
      tmp1 = table[tmpa];
      dest[byte_index - 1] = tmp2;
      tmp2 = table[tmpb];
      s1_word = *(uint16_t *)(s1+ byte_index + 2);
      s2_word = *(uint16_t *)(s2 + byte_index + 2);
      tmpa = (s2_word & 0x00ff) | ((s1_word & 0xff) << 8);
      tmpb = ((s2_word & 0xff00) >> 8) | (s1_word & 0xff00);
      dest[byte_index] = tmp1;
      tmp1 = table[tmpa];
      dest[byte_index + 1] = tmp2;
      tmp2 = table[tmpb];
      byte_index += 4;
    } while (byte_index < nob);
    dest[byte_index - 2] = tmp1;
    dest[byte_index - 1] = tmp2;
    nob += 4;
  }

  do {
    uint8_t tmp1 = s1[byte_index];
    uint8_t tmp2 = s2[byte_index];
    uint16_t index = (tmp1 << 8) | tmp2;
    /* tmp1 in bits 8 - 15, tmp2 in 0 - 7 */
    dest[byte_index] = table[index];
    byte_index++;
  } while (byte_index < nob);
}

void pcunf(Dfmt *output, uint64_t count, const uint8_t *t1)
{
  uint64_t i;
  for (i = 0; i < count; i++) {
    output[i] = t1[output[i]];
  }
}

/* This function isn't currently used
 * It was added around 03/07/2019 but the associated code in hpmi.c
 * is said not to work
 */
uint64_t pcrem(uint64_t p, uint64_t lo, uint64_t hi, const FIELD *f)
{
  hi = reduce(hi, 64, p, f);
  hi += lo;
  if (hi < lo) {
    hi++;
  }
  return hi;
}

/*
 * The code in here has some pitfalls worked around
 * The stuff for bits 16 - 31 (shifts 16 and 24) will suffer from
 * sign extension due to promotion to signed int if it's not
 * separated into a load into uint_64_t followed by a separate shift
 * The code for shifts of 32 or more will suffer internal
 * loss of the high part without the cast (but at least the compiler
 * warns about that whereas the promotion bugs are silent)
 */
void pcxunf(Dfmt *d, const Dfmt *s, uint64_t nob, const uint8_t *t1)
{
  /*
   * This loop fairly much follows the original assembler
   * In particular it handles the input 16 bits at a time
   * The original did this by using 8 bit registers dl and dh
   * which combined as the bottom 16 bits of rdx
   * Clearly in C it would be simpler to write  an extract, look up,
   * shift and put back loop
   */
  while (nob >= 8) {
    uint64_t in64 = *(uint64_t *)s; /* get unary input */
    uint8_t lo_byte = in64 & 0xFF;
    /* tmp is intermediary to stop C type promotion introducing signed ints */
    uint64_t tmp = t1[lo_byte];
    uint64_t out64 = tmp;
    uint8_t hi_byte = (in64 >> 8) & 0xFF;
    out64 |= (t1[hi_byte] << 8);
    in64 >>= 16;
    lo_byte = in64 & 0xFF;
    hi_byte = (in64 >> 8) & 0xFF;
    tmp = t1[lo_byte];
    out64 |= tmp << 16;
    tmp = t1[hi_byte];
    out64 |= tmp << 24;
    in64 >>= 16;
    lo_byte = in64 & 0xFF;
    hi_byte = (in64 >> 8) & 0xFF;
    out64 |= (uint64_t)t1[lo_byte] << 32; /* These ensure we don't lose by shitfing an int out of range */
    out64 |= (uint64_t)t1[hi_byte] << 40;
    in64 >>= 16;
    lo_byte = in64 & 0xFF;
    hi_byte = (in64 >> 8) & 0xFF;
    out64 |= (uint64_t)t1[lo_byte] << 48;
    out64 |= (uint64_t)t1[hi_byte] << 56;
    *(uint64_t *)d ^= out64;
    d += 8;
    s += 8;
    nob -= 8;
  }
  /* Finsh off the stragglers which don't make a full 64 bits */
  while (0 != nob) {
    *(uint8_t *)d ^= t1[*s];
    d++;
    s++;
    nob--;
  }
  return;
}

/*void pcbunf(Dfmt *d, const Dfmt *s, uint64_t nob,
  const uint8_t *t1, const uint8_t *t2)*/
/* rdi, rsi, rdx, rcx, r8 */
void pcbunf(Dfmt *dest, const Dfmt *src, uint64_t nob,
            const uint8_t *t1, const uint8_t *t2)
{
  while (nob >= 17) {
    unsigned int i;
    for (i = 0; i < 8; i++) {
      uint8_t in = src[i];
      uint16_t tmp = dest[i];
      tmp += t1[in] << 8;
      dest[i] = t2[tmp];
    }
    dest += 8;
    src += 8;
    nob -= 8;
  }

  /* Now the stragglers */
  while (nob > 0) {
    uint8_t in = src[0];
    uint16_t tmp = dest[0];
    tmp += t1[in] << 8;
    dest[0] = t2[tmp];
    dest += 1;
    src += 1;
    nob -= 1;
  }
}

/* Functions from pc2.s. These implement characteristic 2 */

/*
 * The next 3 functions are used in the BGrease hpmi function
 * They are for grease type 2 (characteristic 2), machine various types
 * It used SSE2. It's basically xor on a larger scale
 */
void pc2aca(const uint8_t *prog, uint8_t *bv, uint64_t stride)
{
  uint64_t slice_count = 8;
#if NEON
  uint16x8_t xmm[8]; /* Neon registers */
#else
  uint64_t xmm[16]; /* Emulate the SSE or AVX registers */
#endif

  while (slice_count > 0) {
    uint64_t *dest = (uint64_t *)(bv + 256); /* destination starts slot 2 */
    const uint8_t *current_prog = prog; /* restart program */
    unsigned int i;

    memcpy(xmm, dest - 128 / sizeof(*dest), 128);
    for (;;) {
      uint64_t code = *current_prog++; /* get first/next program byte */
      while (code <= 79) {
#if NEON
        uint16x8_t *src;
#else
        uint64_t *src;
#endif
        code <<= 7; /* convert to displacement */
#if NEON
        src = (uint16x8_t *)(bv + code);
        for (i = 0; i < 8; i++) {
          xmm[i] = veorq_u16(xmm[i], src[i]);
        }
#else
        src = (uint64_t *)(bv + code);
        for (i = 0; i < 16; i++) {
          xmm[i] ^= src[i];
        }
#endif
        memcpy(dest, xmm, 128);
        dest += 128 / sizeof(*dest); /* increment destination slot */
        code = *current_prog++; /* get next program byte */
      }

      if (code <= 159) {
        code -= 80;
        code <<= 7; /* multiply by slot size */
        memcpy(xmm, bv + code, 128);
        continue;
      }

      if (code <= 239) {
        code -= 160;
        code <<= 7; /* multiply by slot size */
        dest = (uint64_t *)(bv + code);
        continue;
      }
      break; /* Break the infinite for loop */
    }
    /* anything 240+ is stop at the moment */
    bv += stride; /* add in slice stride */
    slice_count--; /* subtract 1 from slice count */
  }
}

/* The functions pc2acj and pc2acm implement
 * the same algorithm with longer registers,
 * so we get 3 for the price of 1
 */

void pc2acj(const uint8_t *prog, uint8_t *destination, uint64_t stride)
{
  pc2aca(prog, destination, stride);
}

void pc2acm(const uint8_t *prog, uint8_t *destination, uint64_t stride)
{
  pc2aca(prog, destination, stride);
}

void pc2bma(const uint8_t *Afmt, uint8_t *bv, uint8_t *Cfmt)
{
  uint64_t c_skip, a_code;
#if NEON
  uint16x8_t xmm[8]; /* Neon registers */
#else
  uint64_t xmm[16]; /* Emulate the SSE or AVX registers */
#endif

  a_code = *(uint64_t *)Afmt; /* get Afmt word */
  c_skip = a_code & 255;   /* copy for skip */

  while (c_skip != 255) {
    unsigned int i;
    int64_t slice_0 = a_code; /* adslice 0 */
    int64_t slice_1 = a_code; /* adslice 1 */
    int64_t slice_2 = a_code;  /* adslice 2 */
    int64_t slice_3;
    int64_t slice_4 = a_code;  /* adslice 4 */
    int64_t slice_5 = a_code; /* adslice 5 */
    int64_t slice_6 = a_code; /* adslice 6 */
    int64_t slice_7 = a_code; /* adslice 7 */

    c_skip <<= 7; /* 128 * byte Afmt */
    Cfmt += c_skip; /* skip some rows of Cfmt */
    slice_3 = a_code; /* adslice 3 */
    slice_0 >>= 1;   /* shift slice 0 */
    slice_1 >>= 5;   /* shift slice 1 */
    slice_2 >>= 9;    /* shift slice 2 */
    slice_3 >>= 13;  /* shift slice 3 */
    slice_4 >>= 17;   /* shift slice 4 */
    slice_5 >>= 21;  /* shift slice 5 */
    slice_6 >>= 25;  /* shift slice 6 */
    slice_7 >>= 29;  /* shift slice 7 */

    slice_0 &= 1920; /* and slice 0 */
    slice_1 &= 1920; /* and slice 1 */
    slice_2 &= 1920;  /* and slice 2 */
    slice_3 &= 1920; /* and slice 3 */
    slice_4 &= 1920;  /* and slice 4 */
    slice_5 &= 1920; /* and slice 5 */
    slice_6 &= 1920; /* and slice 6 */
    slice_7 &= 1920; /* and slice 7 */

    Afmt += 5; /* point to next Afmt word (yes, this is unaligned) */

    memcpy(xmm, Cfmt, 128);
#if NEON
    for (i = 0; i < 8; i++) {
      xmm[i] = veorq_u16(xmm[i], *(uint16x8_t *)(bv + slice_0 + i * 16));
      xmm[i] = veorq_u16(xmm[i], *(uint16x8_t *)(bv + 2048 + slice_1 + i * 16));
      xmm[i] = veorq_u16(xmm[i], *(uint16x8_t *)(bv + 4096 + slice_2 + i * 16));
      xmm[i] = veorq_u16(xmm[i], *(uint16x8_t *)(bv + 6144 + slice_3 + i * 16));
      xmm[i] = veorq_u16(xmm[i], *(uint16x8_t *)(bv + 8192 + slice_4 + i * 16));
      xmm[i] = veorq_u16(xmm[i], *(uint16x8_t *)(bv + 10240 + slice_5 + i * 16));
      xmm[i] = veorq_u16(xmm[i], *(uint16x8_t *)(bv + 12288 + slice_6 + i * 16));
      xmm[i] = veorq_u16(xmm[i], *(uint16x8_t *)(bv + 14336 + slice_7 + i * 16));
    }
#else
    for (i = 0; i < 16; i++) {
      xmm[i] ^= *(uint64_t *)(bv + slice_0 + i * 8);
    }
    for (i = 0; i < 16; i++) {
      xmm[i] ^= *(uint64_t *)(bv + 2048 + slice_1 + i * 8);
    }
    for (i = 0; i < 16; i++) {
      xmm[i] ^= *(uint64_t *)(bv + 4096 + slice_2 + i * 8);
    }
    for (i = 0; i < 16; i++) {
      xmm[i] ^= *(uint64_t *)(bv + 6144 + slice_3 + i * 8);
    }
    for (i = 0; i < 16; i++) {
      xmm[i] ^= *(uint64_t *)(bv + 8192 + slice_4 + i * 8);
    }
    for (i = 0; i < 16; i++) {
      xmm[i] ^= *(uint64_t *)(bv + 10240 + slice_5 + i * 8);
    }
    for (i = 0; i < 16; i++) {
      xmm[i] ^= *(uint64_t *)(bv + 12288 + slice_6 + i * 8);
    }
    for (i = 0; i < 16; i++) {
      xmm[i] ^= *(uint64_t *)(bv + 14336 + slice_7 + i * 8);
    }
#endif
    memcpy(Cfmt, xmm, 128);

    a_code = *(uint64_t *)Afmt; /* get Afmt word */
    c_skip = a_code;   /* copy for skip */
    c_skip &= 255;  /* mask with 255 */
  }
}

/* The functions pc2bmj and pc2bmm implement
 * the same algorithm with longer registers,
 * so we get 3 for the price of 1 again
 */

void pc2bmj(const uint8_t *a, uint8_t *bv, uint8_t *c)
{
  pc2bma(a, bv, c);
}

void pc2bmm(const uint8_t *a, uint8_t *bv, uint8_t *c)
{
  pc2bma(a, bv, c);
}

/*
 * Now now the same 6 functions for characteristic 3
 * As above, the j and m versions are just calls to the a version
 */
/* Grease functions */

void pc3aca(const uint8_t *prog, uint8_t *bv, uint64_t stride)
{
  uint64_t slice_count = 3;
#if NEON
  uint16x8_t xmm[10]; /* neon 128 bit registers */
#else
  uint64_t xmm[20]; /* Emulate vector registers (extra 4 used differently) */
#endif

  while (slice_count > 0) {
    /* pc3aca1: */
    uint64_t *dest = (uint64_t *)(bv + 256); /* destination starts slot 2 */
    const uint8_t *current_prog = prog; /* restart program */
    unsigned int i;

    memcpy(xmm, dest - 128 / sizeof(*dest), 128);
    for (;;) {
      /* pc3aca2: */
      uint64_t code = *current_prog++; /* get first/next program byte */
      while (code <= 79) {
        /* pc3aca3: */
        uint64_t *src;
        code <<= 7; /* convert to displacement */
        src = (uint64_t *)(bv + code);
        for (i = 0; i < 4; i++) {
#if NEON
          uint64_t offset = i;
          /* Move from (0, 64), (16, 80), (32, 96), (48, 112) */
          xmm[8] = *(uint16x8_t *)(src + offset * 2);
          xmm[9] = *(uint16x8_t *)(src + 8 + offset * 2); /* xmm{8,9} in x86 world*/
          xmm[8] = veorq_u16(xmm[8], xmm[offset]); /*  bu^au -> au   */
          xmm[offset + 4] = veorq_u16(xmm[offset + 4], xmm[9]); /*  at^bt -> bt   */
          xmm[9] = veorq_u16(xmm[9], xmm[8]); /*  au^at -> at   */
          xmm[offset] = veorq_u16(xmm[offset], xmm[offset + 4]); /*  bt^bu -> bu   */
          xmm[9] = vorrq_u16(xmm[9], xmm[offset + 4]); /*  bt|at -> at   */
          xmm[8] = vorrq_u16(xmm[8], xmm[offset]); /*  bu|au -> au   */
          xmm[offset] = xmm[9];
          memcpy(dest + 2 * offset, xmm + offset, 16);
          xmm[offset + 4] = xmm[8];
          memcpy(dest + 2 * offset + 64 / sizeof(*dest), xmm + offset + 4, 16);
#else
          uint64_t offset = 2 * i;
          memcpy(xmm + 16, src + offset, 16);
          memcpy(xmm + 18, src + 8 + offset, 16); /* xmm{8,9} */
          xmm[16] ^= xmm[offset];
          xmm[17] ^= xmm[offset + 1]; /*  bu^au -> au   */
          xmm[offset + 8] ^= xmm[18];
          xmm[offset + 9] ^= xmm[19]; /*  at^bt -> bt   */
          xmm[18] ^= xmm[16];
          xmm[19] ^= xmm[17]; /*  au^at -> at   */
          xmm[offset] ^= xmm[offset + 8];
          xmm[offset + 1] ^= xmm[offset + 9]; /*  bt^bu -> bu   */
          xmm[18] |= xmm[8 + offset];
          xmm[19] |= xmm[9 + offset]; /*  bt|at -> at   */
          xmm[16] |= xmm[offset];
          xmm[17] |= xmm[offset + 1]; /*  bu|au -> au   */
          memcpy(xmm + offset, xmm + 18, 16);
          memcpy(dest + offset, xmm + offset, 16);
          memcpy(xmm + offset + 8, xmm + 16, 16);
          memcpy(dest + offset + 64 / sizeof(*dest), xmm + offset + 8, 16);
#endif
        }
        dest += 128 / sizeof(*dest); /* increment destination slot */
        code = *current_prog++; /* get next program byte */
      }

      if (code <= 159) {
        code -= 80;
        code <<= 7; /* multiply by slot size */
        memcpy(xmm, bv + code, 128);
        continue; /* pc3aca2 */
      }

      if (code <= 239) {
        code -= 160;
        code <<= 7; /* multiply by slot size */
        dest = (uint64_t *)(bv + code);
        continue; /* At pc3aca2 */
      }
      break; /* Break the infinite for loop */
    }
    /* anything 240+ is stop at the moment */
    bv += stride; /* add in slice stride */
    slice_count--; /* subtract 1 from slice count */
    /* continue at pc3aca1 */
  }
}

void pc3acj(const uint8_t *prog, uint8_t *bv, uint64_t stride)
{
  pc3aca(prog, bv, stride);
}

void pc3acm(const uint8_t *prog, uint8_t *bv, uint64_t stride)
{
  pc3aca(prog, bv, stride);
}

/* pc3bma has the same structure as pc2bma, but with different operations in the middle */
void pc3bma(const uint8_t *Afmt, uint8_t *bv, uint8_t *Cfmt)
{
  uint64_t c_skip, a_code;
#if NEON
  uint16x8_t xmm[8];
#else
  uint64_t xmm[16]; /* Emulate the SSE or AVX registers */
#endif
  a_code = *(uint64_t *)Afmt; /* get Afmt word */
  c_skip = a_code & 255;   /* copy for skip */

  while (c_skip != 255) {
#if NEON
    unsigned int j;
#else
    unsigned int i, j;
#endif
    int64_t slice_0 = a_code; /* adslice 0 */
    int64_t slice_1 = a_code; /* adslice 1 */
    int64_t slice_2;
    int64_t aslice_0, aslice_1, aslice_2;

    c_skip <<= 7; /* 128 * byte Afmt */
    Cfmt += c_skip; /* skip some rows of Cfmt */
    slice_2 = a_code; /* adslice 2 */
    slice_0 >>= 2;   /* shift slice 0 */
    slice_1 >>= 10;   /* shift slice 1 */
    slice_2 >>= 18;    /* shift slice 2 */

    slice_0 &= 8128; /* and slice 0 */
    slice_1 &= 8128; /* and slice 1 */
    slice_2 &= 8128;  /* and slice 2 */

    aslice_0 = slice_0 ^ 64;
    aslice_1 = slice_1 ^ 64;
    aslice_2 = slice_2 ^ 64;

    Afmt += 4; /* point to next Afmt word (yes, this is unaligned) */

    for (j = 0; j < 64; j += 16) {
#if NEON
      xmm[0] = *(uint16x8_t *)(Cfmt + j);
      xmm[1] = *(uint16x8_t *)(Cfmt + 64 + j);
      xmm[2] = *(uint16x8_t *)(bv + slice_0 + j);
      xmm[3] = *(uint16x8_t *)(bv + aslice_0 + j);
      xmm[2] = veorq_u16(xmm[2], xmm[0]);
      xmm[1] = veorq_u16(xmm[1], xmm[3]);
      xmm[3] = veorq_u16(xmm[3], xmm[2]);
      xmm[0] = veorq_u16(xmm[0], xmm[1]);
      xmm[3] = vorrq_u16(xmm[3], xmm[1]);
      xmm[2] = vorrq_u16(xmm[2], xmm[0]);
      xmm[0] = *(uint16x8_t *)(bv + 5248 + slice_1 + j);
      xmm[1] = *(uint16x8_t *)(bv + 5248 + aslice_1 + j);
      xmm[0] = veorq_u16(xmm[0], xmm[3]);
      xmm[2] = veorq_u16(xmm[2], xmm[1]);
      xmm[1] = veorq_u16(xmm[1], xmm[0]);
      xmm[3] = veorq_u16(xmm[3], xmm[2]);
      xmm[2] = vorrq_u16(xmm[2], xmm[1]);
      xmm[3] = vorrq_u16(xmm[3], xmm[0]);
      xmm[1] = *(uint16x8_t *)(bv + 10496 + slice_2 + j);
      xmm[0] = *(uint16x8_t *)(bv + 10496 + aslice_2 + j);
      xmm[1] = veorq_u16(xmm[1], xmm[2]);
      xmm[3] = veorq_u16(xmm[3], xmm[0]);
      xmm[0] = veorq_u16(xmm[0], xmm[1]);
      xmm[2] = veorq_u16(xmm[2], xmm[3]);
      xmm[0] = vorrq_u16(xmm[0], xmm[3]);
      xmm[1] = vorrq_u16(xmm[1], xmm[2]);
      *(uint16x8_t *)(Cfmt + j) = xmm[0];
      *(uint16x8_t *)(Cfmt + 64 + j) = xmm[1];
#else
      memcpy(xmm, Cfmt + j, 16);
      memcpy(xmm + 2, Cfmt + 64 + j, 16); /* get 32 bytes of Cfmt */
      memcpy(xmm + 4, bv + slice_0 + j, 16);
      memcpy(xmm + 6, bv + aslice_0 + j, 16);
      for (i = 0; i < 2; i++) {
        xmm[4 + i] ^= xmm[i];
      }
      for (i = 0; i < 2; i++) {
        xmm[2 + i] ^= xmm[6 + i];
      }
      for (i = 0; i < 2; i++) {
        xmm[6 + i] ^= xmm[4 + i];
      }
      for (i = 0; i < 2; i++) {
        xmm[i] ^= xmm[2 + i];
      }
      for (i = 0; i < 2; i++) {
        xmm[6 + i] |= xmm[2 + i];
      }
      for (i = 0; i < 2; i++) {
        xmm[4 + i] |= xmm[i];
      }
      memcpy(xmm, bv + 5248 + slice_1 + j, 16);
      memcpy(xmm + 2, bv + 5248 + aslice_1 + j, 16);
      for (i = 0; i < 2; i++) {
        xmm[i] ^= xmm[6 + i];
      }
      for (i = 0; i < 2; i++) {
        xmm[4 + i] ^= xmm[2 + i];
      }
      for (i = 0; i < 2; i++) {
        xmm[2 + i] ^= xmm[i];
      }
      for (i = 0; i < 2; i++) {
        xmm[6 + i] ^= xmm[4 + i];
      }
      for (i = 0; i < 2; i++) {
        xmm[4 + i] |= xmm[2 + i];
      }
      for (i = 0; i < 2; i++) {
        xmm[6 + i] |= xmm[i];
      }
      memcpy(xmm + 2, bv + 10496 + slice_2 + j, 16);
      memcpy(xmm, bv + 10496 + aslice_2 + j, 16);
      for (i = 0; i < 2; i++) {
        xmm[2 + i] ^= xmm[4 + i];
      }
      for (i = 0; i < 2; i++) {
        xmm[6 + i] ^= xmm[i];
      }
      for (i = 0; i < 2; i++) {
        xmm[i] ^= xmm[2 + i];
      }
      for (i = 0; i < 2; i++) {
        xmm[4 + i] ^= xmm[6 + i];
      }
      for (i = 0; i < 2; i++) {
        xmm[i] |= xmm[6 + i];
      }
      for (i = 0; i < 2; i++) {
        xmm[2 + i] |= xmm[4 + i];
      }
      memcpy(Cfmt + j, xmm, 16);
      memcpy(Cfmt + 64 + j, xmm + 2, 16);
#endif
    }

    a_code = *(uint64_t *)Afmt; /* get Afmt word */
    c_skip = a_code;   /* copy for skip */
    c_skip &= 255;  /* mask with 255 */
  }
}

void pc3bmj(const uint8_t *a, uint8_t *bv, uint8_t *c)
{
  pc3bma(a, bv, c);
}

void pc3bmm(const uint8_t *a, uint8_t *bv, uint8_t *c)
{
  pc3bma(a, bv, c);
}

/*
 * Translations from pc5.s, Add/subtract primes between 5 and 193
 */

#if 0
/* The p shuffle instruction (turns out not to be needed) */
void pshufd(union_128 *mmx, uint8_t control)
{
  unsigned int i;
  union_128 temp;
  for (i = 0; i < 4; i++) {
    unsigned int j = control & 3;
    temp.ints[i] = mmx->ints[j];
    control >>= 2;
  }
  *mmx = temp;
}
#endif

/*
 * The pmullw instruction.
 * Multiply packed 16 bit words inside a 64 bit word
 */
static uint64_t pmullw(uint64_t a, uint64_t b)
{
  union_64_16 x, y;
  unsigned int i;
  x.a = a;
  y.a = b;
  for (i = 0; i < 4; i++) {
    y.words[i] *= x.words[i];
  }
  return y.a;
}

/*
 * The pmulld instruction.
 * Multiply packed 32 bit words inside a 64 bit word
 */
static uint64_t pmulld(uint64_t a, uint64_t b)
{
  union_64_32 x, y;
  unsigned int i;
  x.a = a;
  y.a = b;
  for (i = 0; i < 2; i++) {
    y.doubles[i] *= x.doubles[i];
  }
  return y.a;
}

void pc5aca(const uint8_t *prog, uint8_t *bv, const uint64_t *parms)
{
  uint64_t slice_count;;
  uint64_t xmm[32]; /* Emulate vector registers */
  uint64_t slot_size;
  uint64_t shift;
  xmm[18] = parms[2];
  xmm[19] = xmm[18];
  shift = parms[1];
  xmm[16] = parms[3];
  xmm[17] = xmm[16];
  xmm[22] = parms[0];
  xmm[23] = xmm[22];
  slice_count = parms[6];
  slot_size = parms[4];
  slot_size *= parms[5];

  while (slice_count > 0) {
    /* pc5aca1: */
    uint64_t *dest = (uint64_t *)(bv + 256); /* destination starts slot 2 */
    const uint8_t *current_prog = prog; /* restart program */
    unsigned int i;

    memcpy(xmm, dest - 128 / sizeof(*dest), 128);
    for (;;) {
      /* pc5aca2: */
      uint64_t code = *current_prog++; /* get first/next program byte */
      while (code <= 79) {
        /* pc5aca3: */
        uint64_t *src;
        code <<= 7; /* convert to displacement */
        src = (uint64_t *)(bv + code);
        /* This section does 64 bit packed word instructions on mmx registers */
        /* Eg paddq   16(%rax,%rsi),%xmm1 */
        for (i = 0; i < 16; i++) {
          xmm[i] += src[i];
        }
        for (i = 0; i < 4; i++) {
          uint64_t offset = 4 * i;
          memcpy(xmm + 24, xmm + offset, 16);
          memcpy(xmm + 28, xmm + 2 + offset, 16);
          xmm[24] += xmm[16];
          xmm[28] += xmm[16];
          xmm[25] += xmm[17];
          xmm[29] += xmm[17]; /* add 2^N - p */
          xmm[24] &= xmm[18];
          xmm[28] &= xmm[18];
          xmm[25] &= xmm[19];
          xmm[29] &= xmm[19]; /* and with mask */
          memcpy(xmm + 26, xmm + 24, 16);
          memcpy(xmm + 30, xmm + 28, 16);
          /* Now psrlq   %xmm10,%xmm13 */
          xmm[26] >>= shift;
          xmm[27] >>= shift;
          xmm[30] >>= shift;
          xmm[31] >>= shift; /* subtract 1 if set */
          /* Now psubq   %xmm13,%xmm12, similar to paddq */
          xmm[24] -= xmm[26];
          xmm[25] -= xmm[27];
          xmm[28] -= xmm[30];
          xmm[29] -= xmm[31];
          /* Now pand    %xmm11,%xmm12 */
          xmm[24] &= xmm[22];
          xmm[28] &= xmm[22];
          xmm[25] &= xmm[23];
          xmm[29] &= xmm[23]; /* and with p */
          /* Now psubq   %xmm12,%xmm0 */
          xmm[0 + offset] -= xmm[24];
          xmm[1 + offset] -= xmm[25];
          xmm[2 + offset] -= xmm[28];
          xmm[3 + offset] -= xmm[29]; /* subtract p if need be */
          memcpy(dest + offset, xmm + offset, 32);
        }
        dest += 128 / sizeof(*dest); /* increment destination slot */
        code = *current_prog++; /* get next program byte */
      }
      /* pc5aca4 */
      if (code <= 159) {
        code -= 80;
        code <<= 7; /* multiply by slot size */
        memcpy(xmm, bv + code, 128);
        continue; /* pc5aca2 */
      }
      /* pc5aca5 */
      if (code <= 239) {
        code -= 160;
        code <<= 7; /* multiply by slot size */
        dest = (uint64_t *)(bv + code);
        continue; /* At pc5aca2 */
      }
      break; /* Break the infinite for loop */
    }
    /* anything 240+ is stop at the moment */
    bv += slot_size; /* add in slice stride */
    slice_count--; /* subtract 1 from slice count */
    /* continue at pc5aca1 */
  }
}

void pc5bmwa(const uint8_t *a, uint8_t *bv, uint8_t *c,
             const uint64_t *parms)
{
  uint64_t xmm[32]; /* Emulate vector registers */
  uint64_t slot_size; /* r11 */
  uint64_t *afmt = (uint64_t *)a;
  uint64_t code; /* rax */
  uint64_t skip; /* r9 */
  uint64_t *alcove; /* r8 */
  xmm[16] = parms[2]; /* Mask */
  xmm[17] = xmm[16]; /* The shuffle */
  xmm[18] = parms[1]; /* Shift S */
  xmm[20] = parms[7];
  xmm[21] = xmm[20]; /* 2^S % p */
  xmm[22] = parms[8];
  xmm[23] = xmm[22]; /* bias */
  slot_size = parms[4]; /* size of one slot */
  slot_size *= parms[5]; /* times slots = slice stride */
  code = *afmt; /* first word of Afmt   */
  skip = code & 0xff; /* get skip/terminate   */
  code >>= 1;
  while (255 != skip) {
    /* pc5bmwa1 */
    uint64_t slices = 7;
    unsigned int i, k;
    skip <<= 7; /* multiply by 128      */
    c += skip; /* add into Cfmt addr   */
    alcove = (uint64_t *)bv; /* copy BWA addr to alcove */
    afmt++; /* point to next alcove */
    memcpy(xmm, c, 128); /* 8 16 byte double quad words */
    while (0 != slices) {
      /* pc5bmwa2 */
      uint64_t lo /* r10 */, hi /* r9 */;
      hi = ((code >> 4) & 0x780) / sizeof(*alcove);
      lo = (code & 0x780) / sizeof(*alcove);
      code >>= 8; /* next byte of Afmt */ 
      for (i = 0; i < 16; i++) {
        xmm[i] += alcove[hi + i]; /* add in cauldron */
        xmm[i] -= alcove[lo + i];
      }
      alcove += slot_size / sizeof(*alcove); /* move on to next slice */
      slices--;
    }
    for (k = 0; k < 2; k++) {
      unsigned int offset = k * 8;
      memcpy(xmm + 24, xmm + offset, 64); /* 4 16 byte registers */
      for (i = 0; i < 4; i++) {
        unsigned int j = 2 * i;
        uint64_t shift = xmm[18];
        uint64_t mask32 = (1UL << (32 - shift)) - 1;
        uint64_t mult = 0x0000000100000001UL;
        uint64_t mask64 = mult * mask32;
        xmm[24 + j] &= xmm[16];
        xmm[25 + j] &= xmm[17];
        xmm[j + offset] ^= xmm[24 + j];
        xmm[1 + j + offset] ^= xmm[25 + j];
        /* Shift packed double word right logical */
        xmm[24 + j] >>= shift; /* An unpacked shift */
        xmm[24 + j] &= mask64; /* Now mask out the bits moved from one double to the other */
        xmm[25 + j] >>= shift; /* An unpacked shift */
        xmm[25 + j] &= mask64; /* Now mask out the bits moved from one double to the other */
        /* pmullw: multiply as collections of 16 bits */
        xmm[24 + j] = pmullw(xmm[24 + j], xmm[20]);
        xmm[25 + j] = pmullw(xmm[25 + j], xmm[21]);
        /* paddq %xmm12,%xmm0 13, 14, 15 etc */
        xmm[j + offset] += xmm[24 + j];
        xmm[1 + j + offset] += xmm[25 + j];
        /* paddq %xmm11,%xmm0 11, 11, 11etc */
        xmm[j + offset] += xmm[22];
        xmm[1 + j + offset] += xmm[23];
      }
      memcpy(c + offset * sizeof(uint64_t), xmm + offset, 64); /* Put results back into C format area */
    }
    code = *afmt;
    skip = code & 0xff;
    code >>= 1;
  }
}

void pc5bmwj(const uint8_t *a, uint8_t *bv, uint8_t *c,
             const uint64_t *parms)
{
  pc5bmwa(a, bv, c, parms);
}

void pc5bmdd(const uint8_t *a, uint8_t *bv, uint8_t *c,
             const uint64_t *parms)
{
  uint64_t xmm[32]; /* Emulate vector registers */
  uint64_t slot_size; /* r11 */
  uint64_t *afmt = (uint64_t *)a;
  uint64_t code; /* rax */
  uint64_t skip; /* r9 */
  uint64_t *alcove; /* r8 */
  xmm[16] = parms[2]; /* Mask */
  xmm[17] = xmm[16]; /* The shuffle */
  xmm[18] = parms[1]; /* Shift S */
  xmm[20] = parms[7];
  xmm[21] = xmm[20]; /* 2^S % p */
  xmm[22] = parms[8];
  xmm[23] = xmm[22]; /* bias */
  slot_size = parms[4]; /* size of one slot */
  slot_size *= parms[5]; /* times slots = slice stride */
  code = *afmt; /* first word of Afmt   */
  skip = code & 0xff; /* get skip/terminate   */
  code >>= 1;
  while (255 != skip) {
    /* pc5bmwa1 */
    uint64_t slices = 7;
    unsigned int i, k;
    skip <<= 7; /* multiply by 128      */
    c += skip; /* add into Cfmt addr   */
    alcove = (uint64_t *)bv; /* copy BWA addr to alcove */
    afmt++; /* point to next alcove */
    memcpy(xmm, c, 128); /* 8 16 byte double quad words */
    while (0 != slices) {
      /* pc5bmwa2 */
      uint64_t lo /* r10 */, hi /* r9 */;
      hi = ((code >> 4) & 0x780) / sizeof(*alcove);
      lo = (code & 0x780) / sizeof(*alcove);
      code >>= 8; /* next byte of Afmt */ 
      for (i = 0; i < 16; i++) {
        xmm[i] += alcove[hi + i]; /* add in cauldron */
        xmm[i] -= alcove[lo + i];
      }
      alcove += slot_size / sizeof(*alcove); /* move on to next slice */
      slices--;
    }
    for (k = 0; k < 2; k++) {
      unsigned int offset = k * 8;
      memcpy(xmm + 24, xmm + offset, 64); /* 4 16 byte registers */
      for (i = 0; i < 4; i++) {
        unsigned int j = 2 * i;
        uint64_t shift = xmm[18];
        uint64_t mask32 = (1UL << (32 - shift)) - 1;
        uint64_t mult = 0x0000000100000001UL;
        uint64_t mask64 = mult * mask32;
        xmm[24 + j] &= xmm[16];
        xmm[25 + j] &= xmm[17];
        xmm[j + offset] ^= xmm[24 + j];
        xmm[1 + j + offset] ^= xmm[25 + j];
        /* Shift packed double word right logical */
        xmm[24 + j] >>= shift; /* An unpacked shift */
        xmm[24 + j] &= mask64; /* Now mask out the bits moved from one double to the other */
        xmm[25 + j] >>= shift; /* An unpacked shift */
        xmm[25 + j] &= mask64; /* Now mask out the bits moved from one double to the other */
        /* pmulld: multiply as collections of 16 bits */
        xmm[24 + j] = pmulld(xmm[24 + j], xmm[20]);
        xmm[25 + j] = pmulld(xmm[25 + j], xmm[21]);
        /* paddq %xmm12,%xmm0 13, 14, 15 etc */
        xmm[j + offset] += xmm[24 + j];
        xmm[1 + j + offset] += xmm[25 + j];
        /* paddq %xmm11,%xmm0 11, 11, 11etc */
        xmm[j + offset] += xmm[22];
        xmm[1 + j + offset] += xmm[23];
      }
      memcpy(c + offset * sizeof(uint64_t), xmm + offset, 64); /* Put results back into C format area */
    }
    code = *afmt;
    skip = code & 0xff;
    code >>= 1;
  }
}

void pc5bmdj(const uint8_t *a, uint8_t *bv, uint8_t *c,
             const uint64_t *parms)
{
  pc5bmdd(a, bv, c, parms);
}

void pc5bmdm(const uint8_t *a, uint8_t *bv, uint8_t *c,
             const uint64_t *parms)
{
  pc5bmdd(a, bv, c, parms);
}

/*
 * Translations from pc6.s, code for very large primes (more than 32 bit)
 * There are no tests for this code, as it wasn't working at the time
 * of the latest version
 */
/* input  rdi Afmt     rsi bwa     rdx  Cfmt      %rcx 2^90 % p*/
void pc6bma(const uint8_t *a, uint8_t *bwa, uint8_t *c, uint64_t p90)
{
  uint64_t mask /* r10 */ = (1 << 26) - 1;
  uint64_t *cfmt = (uint64_t *)c; /* r9 */
  uint64_t code = *a;
  /* pc6bma1 */
  /* outer loop One row of A 73 cols of BC*/
  while (255 != code ) {
    unsigned int loop_count = 73; /* cl */
    uint64_t carry /* r13 */ = 0;
    a++; /* Note this makes afmt unaligned */
    code *= 1168;
    cfmt += code / sizeof(*cfmt);
    /* pc6bma2 */
    /* middle loop round cols B cols C */
    while (0 != loop_count) {
      uint64_t lo /* r11 */ = cfmt[0], hi /* r12 */ = cfmt[1], upper, lo_temp;
      uint64_t *block = (uint64_t *)bwa;
      /* Inner loop of 21 */
      unsigned int inner;
      /* pc6bma3 */
      uint64_t * afmt = (uint64_t *)a;
      for (inner = 0; inner < 21; inner++) {
        uint64_t hi_temp;
        lo_temp = lo;
        code = afmt[inner];
        /* 128 bit precise multiply */
        upper = full_multiply(&code, code, block[inner * 168 / sizeof(*cfmt)]);
        lo += code;
        if (lo_temp < lo) {
          hi++; /* Detect carry */
        }
        hi_temp = hi;
        hi += upper; /* The hi part from the mul */
        if (hi < hi_temp) {
          carry++;
        }
      }
      a += 8 * 21;
      /* reduce C back to two words and put back */
      carry = hi << 38;
      /* mul 2^90 mod p */
      upper = full_multiply(&code, carry, p90);
      hi &= mask;
      lo_temp = lo;
      lo += code;
      hi += upper;
      if (lo < lo_temp) {
        hi++;
      }
      /* Stuff with carries and high precision multiply etc */
      cfmt[0] = lo;
      cfmt[1] = hi;
      cfmt += 2;
      bwa += 8;
      loop_count--;
    }
    a += 168; /* Next chunk of Amft */
  }
}
