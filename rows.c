/*
 * $Id: rows.c,v 1.1 2001/08/30 18:31:45 jon Exp $
 *
 * Row manipulation for meataxe
 *
 */

#include <stdio.h>
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
  elts_per_word = bits_in_unsigned_int / nob;
  row_words = (noc + elts_per_word - 1) / elts_per_word;
  for (i = 0; i < elts_per_word; i++) {
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
