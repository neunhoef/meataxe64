/*
 * $Id: clean.c,v 1.2 2001/11/12 13:43:38 jon Exp $
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
#include "matrix.h"
#include "primes.h"
#include "rows.h"
#include "utils.h"

static prime_ops prime_operations = { NULL, NULL, NULL, NULL};

static row_ops row_operations = { NULL, NULL, NULL, NULL};

void clean(unsigned int **m1, unsigned int d1,
           unsigned int **m2, unsigned int d2,
           unsigned int *d_out, int *map,
           unsigned int grease_level, unsigned int prime,
           unsigned int len, unsigned int nob,
           unsigned int start, const char *name)
{
  unsigned int dout = 0;
  unsigned int i;
  unsigned int **grease_rows;
  assert(NULL != m1);
  assert(NULL != m2);
  assert(NULL != d_out);
  assert(NULL != map);
  rows_init(prime, &row_operations);
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
    /* Should only happen if greased */
    for (j = 0; j < count; j++) {
      assert(NULL != m1[i + j]);
      grease_rows[k - 1] = m1[i + j];
      k *= prime;
    }
    /* Now compute grease */
    if (0 == grease_make_rows(count, prime, len)) {
      fprintf(stderr, "%s: unable to compute grease, terminating\n", name);
      exit(1);
    }
    /* Now clean m2 with m1[i, i + count] */
    for (j = 0; j < d2; j++) {
      unsigned int elts = 0;
      assert(NULL != m2[j]);
      /* What if ungreased? */
      for (k = 0; k < count; k++) {
        elts |= (get_element_from_row(nob, map[i + k], m2[j])) << (nob * k);
      }
      if (0 != elts) {
        elts = negate_elements(elts, nob, prime);
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

void echelise(unsigned int **m, unsigned int d,
              unsigned int *d_out, int **map,
              unsigned int ***m_out,
              unsigned int grease_level, unsigned int prime,
              unsigned int len, unsigned int nob,
              unsigned int start, const char *name)
{
  unsigned int dout = 0;
  unsigned int i = 0, j = 0;
  unsigned int **new_mat;
  int *bits; /* The map for m */
  int *new_map;       /* For internal use only */
  assert(NULL != m);
  assert(NULL != d_out);
  assert(NULL != map);
  assert(NULL != m_out);
  primes_init(prime, &prime_operations);
  rows_init(prime, &row_operations);
  if (0 == matrix_malloc(d, (void **)&new_mat)) {
    *d_out = 0; /* Failed */
    return;
  }
  bits = my_malloc(d * sizeof(int));
  new_map = my_malloc(d * sizeof(int));
  memset(new_mat, 0, d * sizeof(void *));
  /* i counts through m. j counts through new_mat */
  while (i < d) {
    unsigned int k = 0, l = 0, n;
    while (k < grease_level && i + l < d) {
      /* Forward clean new row with all rows */
      for (n = 0; n < l; n++) {
        unsigned int elt;
        assert(NULL != m[i + l]);
        elt = get_element_from_row(nob, bits[n], m[i + l]);
        if (0 != elt) {
          elt = (*prime_operations.negate)(elt);
          if (1 == elt) {
            (*row_operations.incer)(new_mat[n], m[i + l], len);
          } else {
            (*row_operations.scaled_adder)(new_mat[n], m[i + l], m[i + l], len, elt);
          }
        }
      }
      if (0 == row_is_zero(m[i + l], len)) {
        /* New linearly independent row */
        unsigned int pos;
        unsigned int elt = first_non_zero(m[i + l], nob, len, &pos);
        assert(0 != elt);
        if (1 != elt) {
          elt = (*prime_operations.invert)(elt);
          (*row_operations.scaler)(m[i + l], m[i + l], len, elt);
        }
        new_mat[j + k] = m[i + l];
        new_map[j + k] = pos;
        bits[i + l] = pos;
        k++;
      } else {
        /* This row not significant */
        bits[i + l] = 0;
      }
      l++; /* Next input row */
    } /* while */
    /* Now back clean new_mat with itself */
    for (n = 1; n < k; n++) {
      unsigned int r;
      for (r = 0; r < n; r++) {
        unsigned int elt = get_element_from_row(nob, new_map[j + n], new_mat[j + r]);
        if (0 != elt) {
          (*row_operations.incer)(new_mat[j + n], new_mat[j + r], len);
        }
      }
    }
    /* Now clean m[0, i] with new_mat[j, j + k] */
    clean(new_mat + j, k, m, i, &n, new_map + j, k, prime, len, nob, start, name);
    /* Now clean m[i + l, d] with new_mat[j, j + k] */
    clean(new_mat + j, k, m + i + l, d - i - l, &n, new_map + j, k, prime, len, nob, start, name);
    i += l;
    j += k;
    dout += k;
  } /* while */
  free(new_map);
  *m_out = new_mat;
  *d_out = dout;
  *map = bits;
}

void clean_init(row_opsp ops)
{
  memcpy(&row_operations, ops, sizeof(row_operations));
}
