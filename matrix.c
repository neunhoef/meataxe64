/*
 * $Id: matrix.c,v 1.7 2001/11/19 19:08:49 jon Exp $
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

void *matrix_malloc(unsigned int nor)
{
  return my_malloc(nor * sizeof(unsigned int *));
}

void matrix_free(void *rows)
{
  assert(NULL != rows);
  free(rows);
}
