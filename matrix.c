/*
 * $Id: matrix.c,v 1.1 2001/09/02 22:16:41 jon Exp $
 *
 * Row manipulation for meataxe
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "utils.h"
#include "matrix.h"

int matrix_malloc(unsigned int nob, unsigned int nor,
                  unsigned int noc, unsigned int ***row)
{
  unsigned int elts_per_word = bits_in_unsigned_int / nob;
  unsigned int row_words = (noc + elts_per_word - 1) / elts_per_word;
  unsigned int row_chars = row_words * sizeof(unsigned int);
  unsigned int i;
  assert(NULL != row);
  *row = malloc(nor * sizeof(unsigned int *));
  if (NULL != *row) {
    for (i = 0; i < nor; i++) {
      (*row)[i] = malloc(row_chars);
      if (NULL == (*row)[i]) {
        unsigned int j;
        for (j = 0; j < i; j++) free((*row)[j]);
        free(*row);
        return 0;
      }
    }
    return 1;
  } else {
    return 0;
  }
}
