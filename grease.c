/*
 * $Id: grease.c,v 1.1 2001/09/02 22:16:41 jon Exp $
 *
 * Functions to grease matrix rows
 *
 */

#include <stdio.h>
#include <assert.h>
#include "utils.h"
#include "elements.h"
#include "grease.h"

int grease(unsigned int nob, unsigned int nor1, unsigned int noc1,
           unsigned int noc2, unsigned int prime, unsigned int *step)
{
  assert(NULL != step);
  NOT_USED(nob);
  NOT_USED(nor1);
  NOT_USED(noc1);
  NOT_USED(noc2);
  NOT_USED(prime);
  *step = 1;
  return 1; /* Trivial implementation for the moment */
}

unsigned int **grease_make_rows(unsigned int **rows2, unsigned int size,
                                unsigned int prime, unsigned int nob,
                                unsigned int col_index, unsigned int *grease_row_count)
{
  assert(NULL != grease_row_count);
  NOT_USED(prime);
  NOT_USED(nob);
  if (1 == size) {
    *grease_row_count = 1;
    return &(rows2[col_index]);
  } else {
    assert(1 == size);
    return NULL;
  }
}

unsigned int grease_get_elt(const unsigned int *row1, unsigned int i,
                            unsigned int size, unsigned int prime,
                            unsigned int nob)
{
  NOT_USED(size);
  NOT_USED(prime);
  return get_element_from_row(nob, i, row1);
}
