/*
 * $Id: rows.c,v 1.8 2001/09/18 23:15:46 jon Exp $
 *
 * Row manipulation for meataxe
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "utils.h"
#include "rows.h"

static int row_add_2(const unsigned int *row1, const unsigned int *row2,
                     unsigned int *row3, unsigned int len)
{
  unsigned int row_words;
  unsigned int i, j;
  assert(0 != len);
  assert(NULL != row1);
  assert(NULL != row2);
  assert(NULL != row3);
  row_words = len / sizeof(unsigned int);
  /* Unroll for efficiency */
  j = row_words / 8;
  for (i = 0; i < j; i++) {
    *(row3++) = *(row1++) ^ *(row2++);
    *(row3++) = *(row1++) ^ *(row2++);
    *(row3++) = *(row1++) ^ *(row2++);
    *(row3++) = *(row1++) ^ *(row2++);
    *(row3++) = *(row1++) ^ *(row2++);
    *(row3++) = *(row1++) ^ *(row2++);
    *(row3++) = *(row1++) ^ *(row2++);
    *(row3++) = *(row1++) ^ *(row2++);
  }
  for (i = j * 8; i < row_words; i++) {
    *(row3++) = *(row1++) ^ *(row2++);
  }
  return 1;
}

static int row_inc_2(const unsigned int *row1,
                     unsigned int *row2, unsigned int len)
{
  unsigned int row_words;
  unsigned int i, j;
  assert(0 != len);
  assert(NULL != row1);
  assert(NULL != row2);
  row_words = len / sizeof(unsigned int);
  /* Unroll for efficiency */
  j = row_words / 8;
  for (i = 0; i < j; i++) {
    *(row2++) ^= *(row1++);
    *(row2++) ^= *(row1++);
    *(row2++) ^= *(row1++);
    *(row2++) ^= *(row1++);
    *(row2++) ^= *(row1++);
    *(row2++) ^= *(row1++);
    *(row2++) ^= *(row1++);
    *(row2++) ^= *(row1++);
  }
  for (i = j * 8; i < row_words; i++) {
    *(row2++) ^= *(row1++);
  }
  return 1;
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

#define ONE_BITS_3 0x55555555
#define TWO_BITS_3 ((ONE_BITS_3) << 1)

#define mod_3_add(a,b,c,d,e,f,g,h) \
  c = (a) + (b); /* Result, not reduced mod 3 */ \
  d = (a) & (b); /* 0, 1 => ignore, 2 => result was 4 = 2 + 2, 3 can't happen */ \
  e = (a) ^ (b); /* 0, 1, 2 => ignore, 3 => result was 3 */ \
  f = d & (TWO_BITS_3); /* Ignore all but 2 + 2 */ \
  g = ((e & (TWO_BITS_3)) >> 1) & (e & (ONE_BITS_3)); /* Pick out 3 case only */ \
  h = g | (f >> 1) /* 01 if answer was 3 or 4, otherwise 0 */

static int row_add_3(const unsigned int *row1, const unsigned int *row2,
                     unsigned int *row3, unsigned int len)
{
  unsigned int row_words;
  unsigned int i;
  assert(0 != len);
  assert(NULL != row1);
  assert(NULL != row2);
  assert(NULL != row3);
  row_words = len / sizeof(unsigned int);
  for (i = 0; i < row_words; i++) {
    unsigned int a, b, c, d, e, f, g, h;
    assert(4 == sizeof(unsigned int));
    a = *(row1++);
    b = *(row2++);
    mod_3_add(a,b,c,d,e,f,g,h);
    *(row3++) = c - (h * 3); /* Reduce mod 3 if needed */
    assert(check_for_3(c - (h * 3)));
  }
  return 1;
}

static int row_inc_3(const unsigned int *row1, unsigned int *row2, unsigned int len)
{
  return row_add_3(row1, row2, row2, len);
}

static int scaled_row_add_3(const unsigned int *row1, const unsigned int *row2,
                            unsigned int *row3, unsigned int len, unsigned int elt)
{
  unsigned int row_words;
  unsigned int i;
  assert(2 == elt);
  assert(0 != len);
  assert(NULL != row1);
  assert(NULL != row2);
  assert(NULL != row3);
  NOT_USED(elt);
  row_words = len / sizeof(unsigned int);
  for (i = 0; i < row_words; i++) {
    unsigned int a, b, c, d, e, f, g, h;
    assert(4 == sizeof(unsigned int));
    a = *(row1++);
    b = *(row2++);
    b = ((b & (ONE_BITS_3)) << 1) | ((b & (TWO_BITS_3)) >> 1); /* Negate b */
    mod_3_add(a,b,c,d,e,f,g,h);
    assert(check_for_3(c - (h * 3)));
    *(row3++) = c - (h * 3); /* Reduce mod 3 if needed */
  }
  return 1;
}

#define ONE_BITS_4 0x55555555
#define TWO_BITS_4 ((ONE_BITS_4) << 1)

static int row_add_4(const unsigned int *row1, const unsigned int *row2,
                     unsigned int *row3, unsigned int len)
{
  unsigned int row_words;
  unsigned int i;
  assert(0 != len);
  assert(NULL != row1);
  assert(NULL != row2);
  assert(NULL != row3);
  row_words = len / sizeof(unsigned int);
  for (i = 0; i < row_words; i++) {
    *(row3++) = *(row1++) ^ *(row2++);
  }
  return 1;
}

static int row_inc_4(const unsigned int *row1, unsigned int *row2, unsigned int len)
{
  return row_add_4(row1, row2, row2, len);
}

static int scaled_row_add_4(const unsigned int *row1, const unsigned int *row2,
                            unsigned int *row3, unsigned int len, unsigned int elt)
{
  unsigned int row_words;
  unsigned int i;
  assert(0 != len);
  assert(NULL != row1);
  assert(NULL != row2);
  assert(NULL != row3);
  assert(2 <= elt && elt <= 3);
  row_words = len / sizeof(unsigned int);
  for (i = 0; i < row_words; i++) {
    unsigned int a, b, c, d, e, f, g, h;
    assert(4 == sizeof(unsigned int));
    a = *(row1++);
    b = *(row2++);
    c = (b & TWO_BITS_4) >> 1;
    d = b & ONE_BITS_4;
    e = c | d; /* Detect non-zero */
    f = b + e * (elt - 1); /* Add, with possible overflow */
    g = c * ((elt == 3) ? 1 : 0); /* Detect 2 or 3 * 3 */
    h = c & d; /* Detect 3  * any */
    b = f - (g |  h) * 3; /* Subtract out the overflows */
    *(row3++) = a ^ b;
  }
  return 1;
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
  c = (a) + (b); /* Result, not reduced mod 3 */ \
  d = (a) & (b) & (FOUR_BITS_5); /* 4 gives the overflow into next digit case */ \
  e = d | (d >> 2); /* Overflow indicator * 5 */ \
  f = c - e; /* Correct overflow into next digit */ \
  g = f & (FOUR_BITS_5); /* Detect possible internal to digit overflows */ \
  h = f & (ONE_BITS_5); /* Detect possible 5s */ \
  j = (f & (TWO_BITS_5)) >> 1; /* Detect possible 6s */ \
  k = (g >> 2) & (h | j); /* Detect 5s and 6s */

static int row_add_5(const unsigned int *row1, const unsigned int *row2,
                     unsigned int *row3, unsigned int len)
{
  unsigned int row_words;
  unsigned int i;
  assert(0 != len);
  assert(NULL != row1);
  assert(NULL != row2);
  assert(NULL != row3);
  row_words = len / sizeof(unsigned int);
  for (i = 0; i < row_words; i++) {
    unsigned int a, b, c, d, e, f, g, h, j, k;
    assert(4 == sizeof(unsigned int));
    a = *(row1++);
    b = *(row2++);
    mod_5_add(a,b,c,d,e,f,g,h,j,k);
    assert(check_for_5(f - (k * 5)));
    *(row3++) = f - (k * 5); /* Reduce mod 5 if needed */
  }
  return 1;
}

static int row_inc_5(const unsigned int *row1, unsigned int *row2, unsigned int len)
{
  return row_add_5(row1, row2, row2, len);
}

static int scaled_row_add_5(const unsigned int *row1, const unsigned int *row2,
                            unsigned int *row3, unsigned int len, unsigned int elt)
{
  unsigned int row_words;
  unsigned int i;
  assert(0 != len);
  assert(NULL != row1);
  assert(NULL != row2);
  assert(NULL != row3);
  assert(2 <= elt && elt <= 4);
  row_words = len / sizeof(unsigned int);
  for (i = 0; i < row_words; i++) {
    unsigned int a, b, c, d, e, f, g, h, j, k;
    assert(4 == sizeof(unsigned int));
    a = *(row1++);
    b = *(row2++);
    switch(elt) {
    case 2:
      c = (b & (FOUR_BITS_5));
      d = (c >> 2) | (c >> 1); /* The 4 -> 3 case */
      e = (b & (TWO_BITS_5));
      f = (b & (ONE_BITS_5));
      g = f & (e >> 1); /* The 3 -> 1 case */
      h = (e | f) ^ (g * 3); /* Reduce to only the 0, 1, 2 case */
      b = d | g | (h << 1); /* 0, 1, 2 -> 0, 2, 4 */
      break;
    case 3:
      c = (b & (FOUR_BITS_5)) >> 1; /* get the 4 -> 2 case */
      d = b & (TWO_BITS_5);
      e = b & (ONE_BITS_5);
      f = (d & (e << 1)) << 1; /* get the 3 -> 4 case */
      g = b ^ (ONE_BITS_5);
      h = g & (d >> 1); /* get the 2 -> 1 case */
      j = (e & ((d ^ (TWO_BITS_5)) >> 1)) * 3; /* The 1 -> 3 case */
      b = c | f | h | j;
      break;
    case 4:
      c = (((b & (FOUR_BITS_5)) >> 2) | ((b & (TWO_BITS_5)) >> 1) | (b & (ONE_BITS_5))) * 5;
      b = b ^ c; /* xor with 5 on non-zero digits */
      c = b & (TWO_BITS_5); /* Detect the 2 bits */
      d = c << 1; /* Move into the 4 bits position */
      b ^= d; /* And normalise back (7 -> 3, 6 -> 2) */
      break;
    default:
      assert(0);
    }
    mod_5_add(a,b,c,d,e,f,g,h,j,k);
    assert(check_for_5(f - (k * 5)));
    *(row3++) = f - (k * 5); /* Reduce mod 5 if needed */
  }
  return 1;
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
  memset(row, 0, len);
}

int rows_init(unsigned int prime, row_opsp ops)
{
  if (2 == prime) {
    ops->adder = &row_add_2;
    ops->incer = &row_inc_2;
    ops->scaled_adder = NULL; /* Should never be called */
    return 1;
  } else if (3 == prime) {
    ops->adder = &row_add_3;
    ops->incer = &row_inc_3;
    ops->scaled_adder = &scaled_row_add_3;
    return 1;
  } else if (4 == prime) {
    ops->adder = &row_add_4;
    ops->incer = &row_inc_4;
    ops->scaled_adder = &scaled_row_add_4;
    return 1;
  } else if (5 == prime) {
    ops->adder = &row_add_5;
    ops->incer = &row_inc_5;
    ops->scaled_adder = &scaled_row_add_5;
    return 1;
  } else {
    return 0;
  }
}
