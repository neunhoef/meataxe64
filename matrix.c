/*
 * $Id: matrix.c,v 1.5 2001/11/07 22:35:27 jon Exp $
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

int matrix_malloc(unsigned int nor, void **rows)
{
  assert(NULL != rows);
  *rows = malloc(nor * sizeof(unsigned int *));
  return (NULL != *rows);
}

void matrix_free(void *rows)
{
  assert(NULL != rows);
  free(rows);
}
