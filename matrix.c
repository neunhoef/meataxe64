/*
 * $Id: matrix.c,v 1.6 2001/11/14 00:07:42 jon Exp $
 *
 * Row manipulation for meataxe
 *
 */

#include "matrix.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "utils.h"
#include "rows.h"

void matrix_malloc(unsigned int nor, void **rows)
{
  assert(NULL != rows);
  *rows = my_malloc(nor * sizeof(unsigned int *));
}

void matrix_free(void *rows)
{
  assert(NULL != rows);
  free(rows);
}
