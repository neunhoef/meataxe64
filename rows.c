/*
 * $Id: rows.c,v 1.4 2001/09/08 12:40:55 jon Exp $
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
                     unsigned int *row3, unsigned int noc)
{
  unsigned int row_words;
  unsigned int i;
  assert(0 != noc);
  assert(NULL != row1);
  assert(NULL != row2);
  assert(NULL != row3);
  row_words = (noc + bits_in_unsigned_int - 1) / bits_in_unsigned_int;
  for (i = 0; i < row_words; i++) {
    *(row3++) = *(row1++) ^ *(row2++);
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

#define ONE_BITS 0x55555555
#define TWO_BITS ((ONE_BITS) << 1)

#define mod_3_add(a,b,c,d,e,f,g,h) \
  c = (a) + (b); /* Result, not reduced mod 3 */ \
  d = (a) & (b); /* 0, 1 => ignore, 2 => result was 4 = 2 + 2, 3 can't happen */ \
  e = (a) ^ (b); /* 0, 1, 2 => ignore, 3 => result was 3 */ \
  f = d & (TWO_BITS); /* Ignore all but 2 + 2 */ \
  g = ((e & (TWO_BITS)) >> 1) & (e & (ONE_BITS)); /* Pick out 3 case only */ \
  h = g | (f >> 1) /* 01 if answer was 3 or 4, otherwise 0 */

static int row_add_3(const unsigned int *row1, const unsigned int *row2,
                     unsigned int *row3, unsigned int noc)
{
  unsigned int row_words;
  unsigned int i;
  unsigned int elts_in_word = bits_in_unsigned_int / 2;
  assert(0 != noc);
  assert(NULL != row1);
  assert(NULL != row2);
  assert(NULL != row3);
  row_words = (noc + elts_in_word - 1) / elts_in_word;
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

static int scaled_row_add_3(const unsigned int *row1, const unsigned int *row2,
                            unsigned int *row3, unsigned int noc, unsigned int elt)
{
  unsigned int row_words;
  unsigned int i;
  unsigned int elts_in_word = bits_in_unsigned_int / 2;
  assert(2 == elt);
  assert(0 != noc);
  assert(NULL != row1);
  assert(NULL != row2);
  assert(NULL != row3);
  NOT_USED(elt);
  row_words = (noc + elts_in_word - 1) / elts_in_word;
  for (i = 0; i < row_words; i++) {
    unsigned int a, b, c, d, e, f, g, h;
    assert(4 == sizeof(unsigned int));
    a = *(row1++);
    b = *(row2++);
    b = ((b & (ONE_BITS)) << 1) | ((b & (TWO_BITS)) >> 1); /* Negate b */
    mod_3_add(a,b,c,d,e,f,g,h);
    assert(check_for_3(c - (h * 3)));
    *(row3++) = c - (h * 3); /* Reduce mod 3 if needed */
  }
  return 1;
}

void row_copy(const unsigned int *row1, unsigned int *row2,
             unsigned int nob, unsigned int noc)
{
  unsigned int elts_per_word;
  unsigned int row_words;
  assert(0 != nob);
  assert(0 != noc);
  assert(NULL != row1);
  assert(NULL != row2);
  elts_per_word = bits_in_unsigned_int / nob;
  row_words = (noc + elts_per_word - 1) / elts_per_word;
  memcpy(row2, row1, row_words * sizeof(unsigned int));
}

int row_malloc(unsigned int nob, unsigned int noc, unsigned int **row)
{
  unsigned int elts_per_word = bits_in_unsigned_int / nob;
  unsigned int row_words = (noc + elts_per_word - 1) / elts_per_word;
  unsigned int row_chars = row_words * sizeof(unsigned int);
  assert(NULL != row);
  *row = malloc(row_chars);
  return (NULL != *row);
}

void row_init(unsigned int *row, unsigned int nob, unsigned int noc)
{
  unsigned int elts_per_word = bits_in_unsigned_int / nob;
  unsigned int row_words = (noc + elts_per_word - 1) / elts_per_word;
  unsigned int row_chars = row_words * sizeof(unsigned int);
  assert(NULL != row);
  memset(row, 0, row_chars);
}

int rows_init(unsigned int prime, row_opsp ops)
{
  if (2 == prime) {
    ops->adder = &row_add_2;
    ops->scaled_adder = NULL; /* Should never be called */
    return 1;
  } else if (3 == prime) {
    ops->adder = &row_add_3;
    ops->scaled_adder = &scaled_row_add_3;
    return 1;
  } else {
    return 0;
  }
}
