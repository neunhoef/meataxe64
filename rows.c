/*
 * $Id: rows.c,v 1.2 2001/09/02 22:16:41 jon Exp $
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

typedef int (*row_adder)(const unsigned int *, const unsigned int *,
                         unsigned int *, unsigned int, unsigned int);

static int row_add_2(const unsigned int *row1, const unsigned int *row2,
                     unsigned int *row3, unsigned int nob, unsigned int noc)
{
  unsigned int elts_per_word;
  unsigned int row_words;
  unsigned int i;
  assert(0 != nob);
  assert(0 != noc);
  assert(NULL != row1);
  assert(NULL != row2);
  assert(NULL != row3);
  elts_per_word = bits_in_unsigned_int / nob;
  row_words = (noc + elts_per_word - 1) / elts_per_word;
  for (i = 0; i < row_words; i++) {
    *(row3++) = *(row1++) ^ *(row2++);
  }
  return 1;
}

int row_add(const unsigned int *row1, const unsigned int *row2,
            unsigned int *row3, unsigned int prime,
            unsigned int nob, unsigned int noc)
{
  if (2 == prime) {
    return row_add_2(row1, row2, row3, nob, noc);
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

