/*
 * $Id: matrix.c,v 1.4 2001/09/20 00:00:16 jon Exp $
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
