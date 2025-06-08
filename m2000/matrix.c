/*
 * $Id: matrix.c,v 1.8 2005/06/22 21:52:53 jon Exp $
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

void *matrix_malloc(u32 nor)
{
  return my_malloc(nor * sizeof(word *));
}

void matrix_free(void *rows)
{
  assert(NULL != rows);
  free(rows);
}
