/*
 * $Id: clean.c,v 1.1 2001/11/07 22:35:27 jon Exp $
 *
 * Cleaning and echilisation for meataxe
 *
 */

#include "clean.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "elements.h"
#include "grease.h"
#include "primes.h"
#include "rows.h"

static prime_ops prime_operations = { NULL, NULL, NULL, NULL};

static row_ops row_operations = { NULL, NULL, NULL, NULL};

void clean(unsigned int **m1, unsigned int d1,
           unsigned int **m2, unsigned int d2,
           unsigned int *d_out, int *map,
           unsigned int grease_level, unsigned int prime,
           unsigned int len,
           unsigned int nob, unsigned int start)
{
  unsigned int dout = 0;
  unsigned int i;
  unsigned int **grease_rows;
  assert(NULL != m1);
  assert(NULL != m2);
  assert(NULL != d_out);
  assert(NULL != map);
  primes_init(prime, &prime_operations);
  if ( 0 < grease_level) {
    grease_init(&row_operations);
    grease_allocate_rows(grease_level, prime, len, &grease_rows, start);
  }
  for (i = 0; i < d1; i += grease_level) {
    unsigned int count = (i + grease_level <= d1) ? grease_level : d1 - i;
    unsigned int j, k = 1;
    if ( 0 < grease_level) {
      grease_init_rows(count, prime);
    }
    /* Now insert the initial elements into grease_rows */
    for (j = 0; j < d2; j++) {
      assert(NULL != m1[i + j]);
      grease_rows[k - 1] = m1[i + j];
      k *= prime;
    }
    /* Now clean m2 with m1[i, i + j] */
    for (j = 0; j < d2; j++) {
      unsigned int elts = 0;
      assert(NULL != m2[j]);
      for (k = 0; k < count; k++) {
        elts |= (m2[j][map[i + k]]) << (nob * k);
      }
      if (0 != elts) {
        elts = (*prime_operations.negate)(elts);
        (*row_operations.incer)(grease_rows[elements_contract(elts, prime, nob) - 1], m2[j], len);
      }
    }
  } /* i */
  for (i = 0; i < d2; i++) {
    assert(NULL != m2[i]);
    if (0 == row_is_zero(m2[i], len)) {
      dout++;
    }
  }
  *d_out = dout;
}

void clean_init(row_opsp ops)
{
  memcpy(&row_operations, ops, sizeof(row_operations));
}
