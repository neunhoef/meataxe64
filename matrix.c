/*
 * $Id: matrix.c,v 1.2 2001/09/12 23:13:04 jon Exp $
 *
 * Row manipulation for meataxe
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "utils.h"
#include "rows.h"
#include "matrix.h"

int matrix_malloc(unsigned int len, unsigned int nor, unsigned int ***row)
{
  unsigned int i;
  assert(NULL != row);
  *row = malloc(nor * sizeof(unsigned int *));
  if (NULL != *row) {
    for (i = 0; i < nor; i++) {
      if (0 == row_malloc(len, (*row)+i)) {
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
