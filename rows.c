/*
 * $Id: rows.c,v 1.3 2001/09/04 23:00:12 jon Exp $
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

int row_add(const unsigned int *row1, const unsigned int *row2,
            unsigned int *row3, unsigned int prime,
            unsigned int nob, unsigned int noc)
{
  NOT_USED(nob);
  if (2 == prime) {
    return row_add_2(row1, row2, row3, noc);
  } else {
    return 0;
  }
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
    return 1;
  } else {
    return 0;
  }
}
