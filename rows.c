/*
 * $Id: rows.c,v 1.27 2004/06/05 21:58:53 jon Exp $
 *
 * Row manipulation for meataxe
 *
 */

#include "rows.h"
#include "primes.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "utils.h"

static prime_ops prime_operations;

int row_is_zero(unsigned int *row, unsigned int len)
{
  while (len > 0) {
    if (0 != *row) {
      return 0;
    } else {
      row++;
      len--;
    }
  }
  return 1;
}

static void row_add_2(const unsigned int *row1, const unsigned int *row2,
                     unsigned int *row3, unsigned int len)
{
  unsigned int i, j;
  const unsigned int *rowa;
  assert(0 != len);
  assert(NULL != row1);
  assert(NULL != row2);
  assert(NULL != row3);
  /* Unroll for efficiency */
  j = len / 8;
  i = j * 8;
  rowa = row1 + i;
  while (row1 < rowa) {
    *(row3++) = *(row1++) ^ *(row2++);
    *(row3++) = *(row1++) ^ *(row2++);
    *(row3++) = *(row1++) ^ *(row2++);
    *(row3++) = *(row1++) ^ *(row2++);
    *(row3++) = *(row1++) ^ *(row2++);
    *(row3++) = *(row1++) ^ *(row2++);
    *(row3++) = *(row1++) ^ *(row2++);
    *(row3++) = *(row1++) ^ *(row2++);
  }
  rowa += len - i;
  while (row1 < rowa) {
    *(row3++) = *(row1++) ^ *(row2++);
  }
}

static void row_inc_2_sub(const unsigned int *row1,
                          unsigned int *row2, unsigned int len)
{
  unsigned int i, j;
  const unsigned int *rowa;
  assert(0 != len);
  assert(NULL != row1);
  assert(NULL != row2);
  /* Unroll for efficiency */
  j = len / 8;
  i = j * 8;
  rowa = row1 + i;
  while (row1 < rowa) {
    *(row2++) ^= *(row1++);
    *(row2++) ^= *(row1++);
    *(row2++) ^= *(row1++);
    *(row2++) ^= *(row1++);
    *(row2++) ^= *(row1++);
    *(row2++) ^= *(row1++);
    *(row2++) ^= *(row1++);
    *(row2++) ^= *(row1++);
  }
  rowa += len - i;
  while (row1 < rowa) {
    *(row2++) ^= *(row1++);
  }
}

static void row_inc_2(const unsigned int *row1,
                      unsigned int *row2, unsigned int len)
{
  if (len > 10) {
    const unsigned int *rowa = row1 + len, *rowb = row1;
    /* Search for first non-zero */
    while (row1 < rowa) {
      if (0 != *row1) {
        unsigned int len1 = row1 - rowb;
        len -= len1;
        row2 += len1;
        row_inc_2_sub(row1, row2, len);
        return;
      }
      row1++;
    }
  } else {
    row_inc_2_sub(row1, row2, len);
  }
}

static unsigned char prod_table_2[0x10000];
static int prod_table_2_init = 0;

static unsigned int row_product_2(const unsigned int *row1, const unsigned int *row2, unsigned int len)
{
  unsigned char res = 0;
  const unsigned int *row = row1 + len;
  assert(NULL != row1);
  assert(NULL != row2);
  while (row1 < row) {
    unsigned int a = *row1;
    if (0 != a) {
      unsigned int b = *row2;
      if (0 != b) {
        unsigned int prod = a & b;
        res ^= prod_table_2[((prod ^ (prod >> 16)) & 0xffff)];
      }
    }
    row1++;
    row2++;
  }
  return res;
}

#ifndef NDEBUG
static int check_for_3(unsigned int a)
{
  unsigned int i;
  for (i = 0; i < 16; i++) {
    if ((a & 3) == 3) {
      return 0;
    }
    a >>= 2;
  }
  return 1;
}
#endif

#define new_add_mod_3 1

#define ONE_BITS_3 0x55555555
#define TWO_BITS_3 ((ONE_BITS_3) << 1)

#if new_add_mod_3
#define mod_3_add(a,b,c,d,e,f,g,h) \
  c = (a) + (b); /* Result, not reduced mod 3 */ \
  d = (a) & (b); /* Non-zero <=> answer is 4 */ \
  e = (a) ^ (b); /* 0, 1, 2 => ignore, 3 => answer is 3 */ \
  f = e & (e << 1); /* Non-zero => answer is 3 */ \
  g = (d | f) & TWO_BITS_3 ; /* Non-zero => answer is 3 or 4 */ \
  h = g | (g >> 1); /* Amount to subtract */ \
  c -= h

#else
#define mod_3_add(a,b,c,d,e,f,g,h) \
  c = (a) + (b); /* Result, not reduced mod 3 */ \
  d = (a) & (b); /* 0, 1 => ignore, 2 => result was 4 = 2 + 2, 3 can't happen */ \
  e = (a) ^ (b); /* 0, 1, 2 => ignore, 3 => result was 3 */ \
  f = d & (TWO_BITS_3); /* Ignore all but 2 + 2 */ \
  g = ((e & (TWO_BITS_3)) >> 1) & (e & (ONE_BITS_3)); /* Pick out 3 case only */ \
  h = g | (f >> 1) /* 01 if answer was 3 or 4, otherwise 0 */
#endif

static void row_add_3(const unsigned int *row1, const unsigned int *row2,
                     unsigned int *row3, unsigned int len)
{
  const unsigned int *rowa;
  assert(0 != len);
  assert(NULL != row1);
  assert(NULL != row2);
  assert(NULL != row3);
  rowa = row1 + len;
  while (row1 < rowa) {
    unsigned int a, b, c, d, e, f, g, h;
    assert(4 == sizeof(unsigned int));
    a = *(row1++);
    b = *(row2++);
#if new_add_mod_3
    mod_3_add(a,b,c,d,e,f,g,h);
    *(row3++) = c;
    assert(check_for_3(c));
#else
    mod_3_add(a,b,c,d,e,f,g,h);
    *(row3++) = c - (h * 3); /* Reduce mod 3 if needed */
    assert(check_for_3(c - (h * 3)));
#endif
  }
}

static void row_inc_3_sub(const unsigned int *row1, unsigned int *row2, unsigned int len)
{
  const unsigned int *rowa;
  assert(0 != len);
  assert(NULL != row1);
  assert(NULL != row2);
  rowa = row1 + len;
  while (row1 < rowa) {
    unsigned int a, b, c, d, e, f, g, h;
    assert(4 == sizeof(unsigned int));
    a = *(row1++);
    b = *(row2);
#if new_add_mod_3
    mod_3_add(a,b,c,d,e,f,g,h);
    *(row2++) = c;
    assert(check_for_3(c));
#else
    mod_3_add(a,b,c,d,e,f,g,h);
    *(row2++) = c - (h * 3); /* Reduce mod 3 if needed */
    assert(check_for_3(c - (h * 3)));
#endif
  }
}

static void row_inc_3(const unsigned int *row1,
                      unsigned int *row2, unsigned int len)
{
  if (len > 10) {
    /* Search for first non-zero */
    const unsigned int *rowa = row1 + len, *rowb = row1;
    while (row1 < rowa) {
      if (0 != *row1) {
        unsigned int len1 = row1 - rowb;
        len -= len1;
        row2 += len1;
        row_inc_3_sub(row1, row2, len);
        return;
      }
      row1++;
    }
  } else {
    row_inc_3_sub(row1, row2, len);
  }
}

#define scale_mod_3(b) (((b) & (ONE_BITS_3)) << 1) | (((b) & (TWO_BITS_3)) >> 1)

static void scaled_row_add_3(const unsigned int *row1, const unsigned int *row2,
                             unsigned int *row3, unsigned int len, unsigned int elt)
{
  const unsigned int *rowa;
  assert(2 == elt);
  assert(0 != len);
  assert(NULL != row1);
  assert(NULL != row2);
  assert(NULL != row3);
  NOT_USED(elt);
  rowa = row1 + len;
  while (row1 < rowa) {
    unsigned int a, b, c, d, e, f, g, h;
    assert(4 == sizeof(unsigned int));
    a = *(row1++);
    b = *(row2++);
    b = scale_mod_3(b); /* Negate b */
#if new_add_mod_3
    mod_3_add(a,b,c,d,e,f,g,h);
    *(row3++) = c;
    assert(check_for_3(c));
#else
    mod_3_add(a,b,c,d,e,f,g,h);
    assert(check_for_3(c - (h * 3)));
    *(row3++) = c - (h * 3); /* Reduce mod 3 if needed */
#endif
  }
}

static void scaled_row_inc_3_sub(const unsigned int *row1, unsigned int *row2,
                                 unsigned int len, unsigned int elt)
{
  const unsigned int *rowa;
  assert(2 == elt);
  assert(0 != len);
  assert(NULL != row1);
  assert(NULL != row2);
  NOT_USED(elt);
  rowa = row1 + len;
  while (row1 < rowa) {
    unsigned int a, b, c, d, e, f, g, h;
    assert(4 == sizeof(unsigned int));
    a = *(row1++);
    b = *(row2);
    a = scale_mod_3(a); /* Negate a */
#if new_add_mod_3
    mod_3_add(a,b,c,d,e,f,g,h);
    *(row2++) = c;
    assert(check_for_3(c));
#else
    mod_3_add(a,b,c,d,e,f,g,h);
    assert(check_for_3(c - (h * 3)));
    *(row2++) = c - (h * 3); /* Reduce mod 3 if needed */
#endif
  }
}

static void scaled_row_inc_3(const unsigned int *row1, unsigned int *row2,
                             unsigned int len, unsigned int elt)
{
  if (len > 10) {
    /* Search for first non-zero */
    const unsigned int *rowa = row1 + len, *rowb = row1;
    while (row1 < rowa) {
      if (0 != *row1) {
        unsigned int len1 = row1 - rowb;
        len -= len1;
        row2 += len1;
        scaled_row_inc_3_sub(row1, row2, len, elt);
        return;
      }
      row1++;
    }
  } else {
    scaled_row_inc_3_sub(row1, row2, len, elt);
  }
}

static void row_scale_3(const unsigned int *row1, unsigned int *row2,
                        unsigned int len, unsigned int elt)
{
  const unsigned int *rowa;
  assert(2 == elt);
  assert(0 != len);
  assert(NULL != row1);
  assert(NULL != row2);
  NOT_USED(elt);
  rowa = row1 + len;
  while (row1 < rowa) {
    unsigned int a;
    assert(4 == sizeof(unsigned int));
    a = *(row1++);
    *(row2++) = scale_mod_3(a);
  }
}

static void row_scale_in_place_3(unsigned int *row,
                                 unsigned int len, unsigned int elt)
{
  const unsigned int *rowa;
  assert(2 == elt);
  assert(0 != len);
  assert(NULL != row);
  NOT_USED(elt);
  rowa = row + len;
  while (row < rowa) {
    unsigned int a;
    assert(4 == sizeof(unsigned int));
    a = *(row);
    *(row++) = scale_mod_3(a);
  }
}

static unsigned int prod_table_3[0x10000];

static unsigned int row_product_3(const unsigned int *row1, const unsigned int *row2, unsigned int len)
{
  unsigned int res = 0;
  const unsigned int *row = row1 + len;
  assert(NULL != row1);
  assert(NULL != row2);
  while (row1 < row) {
    unsigned int a = *row1;
    if (0 != a) {
      unsigned int b = *row2;
      while (0 != b && 0 != a) {
        unsigned int prod = ((a & 0xff) << 8) | (b & 0xff);
        res += prod_table_3[prod];
        b >>= 8;
        a >>= 8;
      }
    }
    res %= 3;
    row1++;
    row2++;
  }
  return res;
}

#define ONE_BITS_4 0x55555555
#define TWO_BITS_4 ((ONE_BITS_4) << 1)

#if 0 /* These the same as GF(2) */
static void row_add_4(const unsigned int *row1, const unsigned int *row2,
                      unsigned int *row3, unsigned int len)
{
  unsigned int i;
  assert(0 != len);
  assert(NULL != row1);
  assert(NULL != row2);
  assert(NULL != row3);
  for (i = 0; i < len; i++) {
    *(row3++) = *(row1++) ^ *(row2++);
  }
}

static void row_inc_4(const unsigned int *row1, unsigned int *row2, unsigned int len)
{
  unsigned int i;
  assert(0 != len);
  assert(NULL != row1);
  assert(NULL != row2);
  for (i = 0; i < len; i++) {
    *(row2++) ^= *(row1++);
  }
}
#endif

#define scale_mod_4_X(b,c,d) \
    c = (b) & ONE_BITS_4; \
    d = (b) & TWO_BITS_4; \
    b = ((c << 1) ^ (d)) | ((d) >> 1)

#define scale_mod_4_X_plus_1(b,c,d) \
    c = (b) & ONE_BITS_4; \
    d = (b) & TWO_BITS_4; \
    b = (c << 1) | (c ^ ((d) >> 1))

#define new_scale_mod_4 1

#define scale_mod_4(b,c,d,e,f,g,h,elt) \
    c = ((b) & TWO_BITS_4) >> 1; \
    d = (b) & ONE_BITS_4; \
    e = c | d; /* Detect non-zero */ \
    f = (b) + e * (elt - 1); /* Add, with possible overflow */ \
    g = c * ((elt == 3) ? 1 : 0); /* Detect 2 or 3 * 3 */ \
    h = c & d; /* Detect 3  * any */ \
    b = f - (g |  h) * 3 /* Subtract out the overflows */ 

static void scaled_row_add_4(const unsigned int *row1, const unsigned int *row2,
                             unsigned int *row3, unsigned int len, unsigned int elt)
{
  unsigned int i;
  assert(0 != len);
  assert(NULL != row1);
  assert(NULL != row2);
  assert(NULL != row3);
  assert(2 <= elt && elt <= 3);
  for (i = 0; i < len; i++) {
#if new_scale_mod_4
    unsigned int a, b, c, d;
#else
    unsigned int a, b, c, d, e, f, g, h;
#endif
    assert(4 == sizeof(unsigned int));
    a = *(row1++);
    b = *(row2++);
#if new_scale_mod_4
    if (2 == elt) {
      scale_mod_4_X(b,c,d);
    } else {
      scale_mod_4_X_plus_1(b,c,d);
    }
#else
    scale_mod_4(b,c,d,e,f,g,h,elt);
#endif
    *(row3++) = a ^ b;
  }
}

static void scaled_row_inc_4_sub(const unsigned int *row1, unsigned int *row2,
                                 unsigned int len, unsigned int elt)
{
  unsigned int i;
  assert(0 != len);
  assert(NULL != row1);
  assert(NULL != row2);
  assert(2 <= elt && elt <= 3);
  for (i = 0; i < len; i++) {
#if new_scale_mod_4
    unsigned int a, b, c, d;
#else
    unsigned int a, b, c, d, e, f, g, h;
#endif
    assert(4 == sizeof(unsigned int));
    a = *(row1++);
    b = *(row2);
#if new_scale_mod_4
    if (2 == elt) {
      scale_mod_4_X(a,c, d);
    } else {
      scale_mod_4_X_plus_1(a,c, d);
    }
#else
    scale_mod_4(a,c,d,e,f,g,h,elt);
#endif
    *(row2++) = a ^ b;
  }
}

static void scaled_row_inc_4(const unsigned int *row1, unsigned int *row2,
                             unsigned int len, unsigned int elt)
{
  if (len > 10) {
    const unsigned int *rowa = row1 + len, *rowb = row1;
    /* Search for first non-zero */
    while (row1 < rowa) {
      if (0 != *row1) {
        unsigned int len1 = row1 - rowb;
        len -= len1;
        row2 += len1;
        scaled_row_inc_4_sub(row1, row2, len, elt);
        return;
      }
      row1++;
    }
  } else {
    scaled_row_inc_4_sub(row1, row2, len, elt);
  }
}

static void row_scale_4(const unsigned int *row1, unsigned int *row2,
                       unsigned int len, unsigned int elt)
{
  unsigned int i;
  assert(2 <= elt && elt <= 3);
  assert(0 != len);
  assert(NULL != row1);
  assert(NULL != row2);
  NOT_USED(elt);
  for (i = 0; i < len; i++) {
#if new_scale_mod_4
    unsigned int b, c, d;
#else
    unsigned int b, c, d, e, f, g, h;
#endif
    assert(4 == sizeof(unsigned int));
    b = *(row1++);
#if new_scale_mod_4
    if (2 == elt) {
      scale_mod_4_X(b,c, d);
    } else {
      scale_mod_4_X_plus_1(b,c, d);
    }
#else
    scale_mod_4(b,c,d,e,f,g,h,elt);
#endif
    *(row2++) = b;
  }
}

static void row_scale_in_place_4(unsigned int *row,
                                 unsigned int len, unsigned int elt)
{
  unsigned int i;
  assert(2 <= elt && elt <= 3);
  assert(0 != len);
  assert(NULL != row);
  for (i = 0; i < len; i++) {
    unsigned int b, c, d, e, f, g, h;
    assert(4 == sizeof(unsigned int));
    b = *(row);
    scale_mod_4(b,c,d,e,f,g,h,elt);
    *(row++) = b;
  }
}

static unsigned int prod_table_4[0x10000];

static unsigned int row_product_4(const unsigned int *row1, const unsigned int *row2, unsigned int len)
{
  unsigned int res = 0;
  const unsigned int *row = row1 + len;
  assert(NULL != row1);
  assert(NULL != row2);
  while (row1 < row) {
    unsigned int a = *row1;
    if (0 != a) {
      unsigned int b = *row2;
      while (0 != b && 0 != a) {
        unsigned int prod = ((a & 0xff) << 8) | (b & 0xff);
        res ^= prod_table_4[(prod)];
        b >>= 8;
        a >>= 8;
      }
    }
    row1++;
    row2++;
  }
  return res;
}

#ifndef NDEBUG
static int check_for_5(unsigned int a)
{
  unsigned int i;
  for (i = 0; i < 10; i++) {
    if ((a & 7) >= 5) {
      return 0;
    }
    a >>= 3;
  }
  return 1;
}
#endif

#define FOUR_BITS_5 04444444444 /* Detect overflow into next digit mod 5 */
#define ONE_BITS_5 01111111111 /* Detect bit 0 one in digit */
#define TWO_BITS_5 ((ONE_BITS_5) << 1) /* Detect bit 1 one in digit */

#define mod_5_add(a,b,c,d,e,f,g,h,j,k) \
  c = (a) + (b); /* Result, not reduced mod 5 */ \
  d = (a) & (b) & (FOUR_BITS_5); /* 4 gives the overflow into next digit case */ \
  e = d | (d >> 2); /* Overflow indicator * 5 */ \
  f = c - e; /* Correct overflow into next digit */ \
  g = f & (FOUR_BITS_5); /* Detect possible internal to digit overflows */ \
  h = f & (ONE_BITS_5); /* Detect possible 5s */ \
  j = (f & (TWO_BITS_5)) >> 1; /* Detect possible 6s */ \
  k = (g >> 2) & (h | j); /* Detect 5s and 6s */

static void row_add_5(const unsigned int *row1, const unsigned int *row2,
                     unsigned int *row3, unsigned int len)
{
  const unsigned int *rowa;
  assert(0 != len);
  assert(NULL != row1);
  assert(NULL != row2);
  assert(NULL != row3);
  rowa = row1 + len;
  while (row1 < rowa) {
    unsigned int a, b, c, d, e, f, g, h, j, k;
    assert(4 == sizeof(unsigned int));
    a = *(row1++);
    b = *(row2++);
    mod_5_add(a,b,c,d,e,f,g,h,j,k);
    assert(check_for_5(f - (k * 5)));
    *(row3++) = f - (k * 5); /* Reduce mod 5 if needed */
  }
}

static void row_inc_5_sub(const unsigned int *row1, unsigned int *row2, unsigned int len)
{
  const unsigned int *rowa;
  assert(0 != len);
  assert(NULL != row1);
  assert(NULL != row2);
  rowa = row1 + len;
  while (row1 < rowa) {
    unsigned int a, b, c, d, e, f, g, h, j, k;
    assert(4 == sizeof(unsigned int));
    a = *(row1++);
    b = *(row2);
    mod_5_add(a,b,c,d,e,f,g,h,j,k);
    assert(check_for_5(f - (k * 5)));
    *(row2++) = f - (k * 5); /* Reduce mod 5 if needed */
  }
}

static void row_inc_5(const unsigned int *row1,
                      unsigned int *row2, unsigned int len)
{
  if (len > 10) {
    /* Search for first non-zero */
    const unsigned int *rowa = row1 + len, *rowb = row1;
    while (row1 < rowa) {
      if (0 != *row1) {
        unsigned int len1 = row1 - rowb;
        len -= len1;
        row2 += len1;
        row_inc_5_sub(row1, row2, len);
        return;
      }
      row1++;
    }
  } else {
    row_inc_5_sub(row1, row2, len);
  }
}

#define scale_mod_5(b,d,c,e,f,g,h,j,elt) \
    switch(elt) { \
    case 2: \
      c = (b & (FOUR_BITS_5)); \
      d = (c >> 2) | (c >> 1); /* The 4 -> 3 case */ \
      e = (b & (TWO_BITS_5)); \
      f = (b & (ONE_BITS_5)); \
      g = f & (e >> 1); /* The 3 -> 1 case */ \
      h = (e | f) ^ (g * 3); /* Reduce to only the 0, 1, 2 case */ \
      b = d | g | (h << 1); /* 0, 1, 2 -> 0, 2, 4 */ \
      break; \
    case 3: \
      c = (b & (FOUR_BITS_5)) >> 1; /* get the 4 -> 2 case */ \
      d = b & (TWO_BITS_5); \
      e = b & (ONE_BITS_5); \
      f = (d & (e << 1)) << 1; /* get the 3 -> 4 case */ \
      g = b ^ (ONE_BITS_5); \
      h = g & (d >> 1); /* get the 2 -> 1 case */ \
      j = (e & ((d ^ (TWO_BITS_5)) >> 1)) * 3; /* The 1 -> 3 case */ \
      b = c | f | h | j; \
      break; \
    case 4: \
      c = (((b & (FOUR_BITS_5)) >> 2) | ((b & (TWO_BITS_5)) >> 1) | (b & (ONE_BITS_5))) * 5; \
      b = b ^ c; /* xor with 5 on non-zero digits */ \
      c = b & (TWO_BITS_5); /* Detect the 2 bits */ \
      d = c << 1; /* Move into the 4 bits position */ \
      b ^= d; /* And normalise back (7 -> 3, 6 -> 2) */ \
      break; \
    default: \
      assert(0); \
    }

static void scaled_row_add_5(const unsigned int *row1, const unsigned int *row2,
                             unsigned int *row3, unsigned int len, unsigned int elt)
{
  const unsigned int *rowa;
  assert(0 != len);
  assert(NULL != row1);
  assert(NULL != row2);
  assert(NULL != row3);
  assert(2 <= elt && elt <= 4);
  rowa = row1 + len;
  while (row1 < rowa) {
    unsigned int a, b, c, d, e, f, g, h, j, k;
    assert(4 == sizeof(unsigned int));
    a = *(row1++);
    b = *(row2++);
    scale_mod_5(b,d,c,e,f,g,h,j,elt);
    mod_5_add(a,b,c,d,e,f,g,h,j,k);
    assert(check_for_5(f - (k * 5)));
    *(row3++) = f - (k * 5); /* Reduce mod 5 if needed */
  }
}

static void scaled_row_inc_5_sub(const unsigned int *row1, unsigned int *row2,
                                 unsigned int len, unsigned int elt)
{
  const unsigned int *rowa;
  assert(0 != len);
  assert(NULL != row1);
  assert(NULL != row2);
  assert(2 <= elt && elt <= 4);
  rowa = row1 + len;
  while (row1 < rowa) {
    unsigned int a, b, c, d, e, f, g, h, j, k;
    assert(4 == sizeof(unsigned int));
    a = *(row1++);
    b = *(row2);
    scale_mod_5(a,d,c,e,f,g,h,j,elt);
    mod_5_add(a,b,c,d,e,f,g,h,j,k);
    assert(check_for_5(f - (k * 5)));
    *(row2++) = f - (k * 5); /* Reduce mod 5 if needed */
  }
}

static void scaled_row_inc_5(const unsigned int *row1, unsigned int *row2,
                             unsigned int len, unsigned int elt)
{
  if (len > 10) {
    /* Search for first non-zero */
    const unsigned int *rowa = row1 + len, *rowb = row1;
    while (row1 < rowa) {
      if (0 != *row1) {
        unsigned int len1 = row1 - rowb;
        len -= len1;
        row2 += len1;
        scaled_row_inc_5_sub(row1, row2, len, elt);
        return;
      }
      row1++;
    }
  } else {
    scaled_row_inc_5_sub(row1, row2, len, elt);
  }
}

static void row_scale_5(const unsigned int *row1, unsigned int *row2,
                       unsigned int len, unsigned int elt)
{
  const unsigned int *rowa;
  assert(0 != len);
  assert(NULL != row1);
  assert(NULL != row2);
  assert(2 <= elt && elt <= 4);
  rowa = row1 + len;
  while (row1 < rowa) {
    unsigned int b, c, d, e, f, g, h, j;
    assert(4 == sizeof(unsigned int));
    b = *(row1++);
    scale_mod_5(b,d,c,e,f,g,h,j,elt);
    *(row2++) = b;
  }
}

static void row_scale_in_place_5(unsigned int *row,
                                 unsigned int len, unsigned int elt)
{
  const unsigned int *rowa;
  assert(0 != len);
  assert(NULL != row);
  assert(2 <= elt && elt <= 4);
  rowa = row + len;
  while (row < rowa) {
    unsigned int b, c, d, e, f, g, h, j;
    assert(4 == sizeof(unsigned int));
    b = *(row);
    scale_mod_5(b,d,c,e,f,g,h,j,elt);
    *(row++) = b;
  }
}

static unsigned int row_product_5(const unsigned int *row1, const unsigned int *row2, unsigned int len)
{
  unsigned int res = 0;
  const unsigned int *row = row1 + len;
  assert(NULL != row1);
  assert(NULL != row2);
  while (row1 < row) {
    unsigned int a = *row1;
    if (0 != a) {
      unsigned int b = *row2;
      while (0 != b && 0 != a) {
        unsigned int prod = (a & 0x7) * (b & 0x7);
        res += prod;
        b >>= 3;
        a >>= 3;
      }
    }
    res %= 5;
    row1++;
    row2++;
  }
  return res;
}

#include "9.c"

/* This macro only references its parameters once */
#define word_add_9(in1,in2,out) \
{ \
  unsigned int a = 0, b = in1, c = in2, j = 0; \
  while (0 != b || 0 != c) { \
    unsigned int d = b & 0xff, e = c & 0xff, f; \
    f = add_table_9[(d << 8) | e]; \
    a |= f << j; \
    b >>= 8; \
    c >>= 8; \
    j += 8; \
  } \
  out = a; \
}

static void row_add_9(const unsigned int *row1, const unsigned int *row2,
                     unsigned int *row3, unsigned int len)
{
  unsigned int i;
  assert(0 != len);
  assert(NULL != row1);
  assert(NULL != row2);
  assert(NULL != row3);
  for (i = 0; i < len; i++) {
    word_add_9(*(row1++), *(row2++), *(row3++));
  }
}

static void row_inc_9(const unsigned int *row1,
                      unsigned int *row2, unsigned int len)
{
  if (len > 10) {
    /* Search for first non-zero */
    while (len > 0) {
      if (0 != *row1) {
        row_add_9(row1, row2, row2, len);
        return;
      }
      row1++;
      row2++;
      len--;
    }
  } else {
    row_add_9(row1, row2, row2, len);
  }
}

/* This macro only references its parameters once */
#define char_scale_9(in,out,n) \
{ \
  assert(2 <= n && n < 9); \
  out = scale_table_9[n - 2][in]; \
}

/* This macro only references its parameters once */
#define word_scale_9_with_tab(in,out) \
{ \
  unsigned int res = 0, idx = 0, icopy = in; \
  while (0 != icopy) { \
    res |= (tab[icopy & 0xff]) << idx; \
    icopy >>= 8; \
    idx += 8; \
  } \
  out = res; \
}

/* This macro only references its parameters once */
#define word_scale_9(in,out,n) \
{ \
  unsigned int *tab; \
  assert(2 <= (n) && (n) < 9); \
  tab = scale_table_9[n - 2]; \
  word_scale_9_with_tab(in,out);\
}

static void scaled_row_add_9(const unsigned int *row1, const unsigned int *row2,
                             unsigned int *row3, unsigned int len, unsigned int elt)
{
  unsigned int i, *tab;
  assert(0 != len);
  assert(NULL != row1);
  assert(NULL != row2);
  assert(NULL != row3);
  assert(2 <= elt && elt < 9);
  tab = scale_table_9[elt - 2];
  for (i = 0; i < len; i++) {
    unsigned int out;
    word_scale_9_with_tab(*(row2++),out);
    word_add_9(*(row1++),out,*(row3++));
  }
}

static void scaled_row_inc_9_sub(const unsigned int *row1, unsigned int *row2,
                                 unsigned int len, unsigned int elt)
{
  unsigned int i, *tab;
  assert(0 != len);
  assert(NULL != row1);
  assert(NULL != row2);
  assert(2 <= elt && elt < 9);
  tab = scale_table_9[elt - 2];
  for (i = 0; i < len; i++) {
    unsigned int out;
    word_scale_9_with_tab(*(row1++),out);
    word_add_9(*(row2),out,*(row2));
    row2++;
  }
}

static void scaled_row_inc_9(const unsigned int *row1, unsigned int *row2,
                             unsigned int len, unsigned int elt)
{
  if (len > 10) {
    /* Search for first non-zero */
    while (len > 0) {
      if (0 != *row1) {
        scaled_row_inc_9_sub(row1, row2, len, elt);
        return;
      }
      row1++;
      row2++;
      len--;
    }
  } else {
    scaled_row_inc_9_sub(row1, row2, len, elt);
  }
}

static void row_scale_9(const unsigned int *row1, unsigned int *row2,
                        unsigned int len, unsigned int elt)
{
  unsigned int i, *tab;
  assert(0 != len);
  assert(NULL != row1);
  assert(NULL != row2);
  assert(2 <= elt && elt < 9);
  tab = scale_table_9[elt - 2];
  for (i = 0; i < len; i++) {
    unsigned int out;
    word_scale_9_with_tab(*(row1++),out);
    *(row2++) = out;
  }
}

static void row_scale_in_place_9(unsigned int *row,
                                 unsigned int len, unsigned int elt)
{
  unsigned int i, *tab;
  assert(0 != len);
  assert(NULL != row);
  assert(2 <= elt && elt < 9);
  tab = scale_table_9[elt - 2];
  for (i = 0; i < len; i++) {
    unsigned int out;
    word_scale_9_with_tab(*(row),out);
    *(row++) = out;
  }
}

static unsigned int row_product_9(const unsigned int *row1, const unsigned int *row2, unsigned int len)
{
  unsigned int res = 0;
  const unsigned int *row = row1 + len;
  assert(NULL != row1);
  assert(NULL != row2);
  while (row1 < row) {
    unsigned int a = *row1;
    if (0 != a) {
      unsigned int b = *row2;
      while (0 != b && 0 != a) {
        unsigned int prod = ((a & 0xf) << 4) | (b & 0xf);
        res = (*prime_operations.add)(res, prod_table_9[prod]);
        b >>= 4;
        a >>= 4;
      }
    }
    row1++;
    row2++;
  }
  return res;
}

void row_copy(const unsigned int *row1, unsigned int *row2,
              unsigned int len)
{
  assert(0 != len);
  assert(NULL != row1);
  assert(NULL != row2);
  memcpy(row2, row1, len);
}

void row_init(unsigned int *row, unsigned int len)
{
  assert(NULL != row);
  memset(row, 0, len * sizeof(unsigned int));
}

int rows_init(unsigned int prime, row_opsp ops)
{
  unsigned int i;
  switch (prime) {
  case 2:
    ops->adder = &row_add_2;
    ops->incer = &row_inc_2;
    ops->scaled_adder = NULL; /* Should never be called */
    ops->scaled_incer = NULL; /* Should never be called */
    ops->scaler = NULL; /* Should never be called */
    ops->scaler_in_place = NULL; /* Should never be called */
    ops->product = &row_product_2;
    if (0 == prod_table_2_init) {
      for (i = 0; i < 0x10000; i++) {
        unsigned char prod = 0;
        unsigned int j = i;
        while (0 != j) {
          prod ^= 1;
          j &= (j - 1);
        }
        prod_table_2[i] = prod;
      }
      prod_table_2_init = 1;
    }
    break;
  case 3:
    ops->adder = &row_add_3;
    ops->incer = &row_inc_3;
    ops->scaled_adder = &scaled_row_add_3;
    ops->scaled_incer = &scaled_row_inc_3;
    ops->scaler = &row_scale_3;
    ops->scaler_in_place = &row_scale_in_place_3;
    ops->product = row_product_3;
    for (i = 0; i < 0x10000; i++) {
      unsigned int prod = 0;
      unsigned int j = i & 0xff;
      unsigned int k = (i & 0xff00) >> 8;
      while (0 != j && 0 != k) {
        unsigned int l = j & 3;
        unsigned int m = k & 3;
        switch (4 * l + m) {
        case 0x0:
        case 0x1:
        case 0x2:
        case 0x3:
        case 0x4:
        case 0x7:
        case 0x8:
        case 0xb:
        case 0xc:
        case 0xd:
        case 0xe:
        case 0xf:
          break;
        case 0x6:
        case 0x9:
          prod += 2;
          break;
        case 0x5:
        case 0xa:
          prod += 1;
          break;
        }
        j >>= 2;
        k >>= 2;
      }
      prod_table_3[i] = prod % 3;
    }
    break;
  case 4:
    ops->adder = &row_add_2;
    ops->incer = &row_inc_2;
    ops->scaled_adder = &scaled_row_add_4;
    ops->scaled_incer = &scaled_row_inc_4;
    ops->scaler = &row_scale_4;
    ops->scaler_in_place = &row_scale_in_place_4;
    ops->product = &row_product_4;
    for (i = 0; i < 0x10000; i++) {
      unsigned int prod = 0;
      unsigned int j = i & 0xff;
      unsigned int k = (i & 0xff00) >> 8;
      while (0 != j && 0 != k) {
        unsigned int l = j & 3;
        unsigned int m = k & 3;
        switch (4 * l + m) {
        case 0x0:
        case 0x1:
        case 0x2:
        case 0x3:
        case 0x4:
        case 0x8:
        case 0xc:
          break;
        case 0x5:
        case 0xb:
        case 0xe:
          prod ^= 1;
          break;
        case 0x6:
        case 0x9:
        case 0xf:
          prod ^= 2;
          break;
        case 0x7:
        case 0xa:
        case 0xd:
          prod ^= 1;
          break;
        }
        j >>= 2;
        k >>= 2;
      }
      prod_table_4[i] = prod;
    }
    break;
  case 5:
    ops->adder = &row_add_5;
    ops->incer = &row_inc_5;
    ops->scaled_adder = &scaled_row_add_5;
    ops->scaled_incer = &scaled_row_inc_5;
    ops->scaler = &row_scale_5;
    ops->scaler_in_place = &row_scale_in_place_5;
    ops->product = &row_product_5;
    break;
  case 9:
    if (0 == primes_init(9, &prime_operations)) {
      return 0;
    }
    ops->adder = &row_add_9;
    ops->incer = &row_inc_9;
    ops->scaled_adder = &scaled_row_add_9;
    ops->scaled_incer = &scaled_row_inc_9;
    ops->scaler = &row_scale_9;
    ops->scaler_in_place = &row_scale_in_place_9;
    ops->product = &row_product_9;
    break;
  default:
    assert(0);
    return 0;
  }
  return 1;
}
