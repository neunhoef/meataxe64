/*
 * $Id: clean.c,v 1.13 2002/02/27 19:06:17 jon Exp $
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
#include "memory.h"
#include "primes.h"
#include "rows.h"
#include "utils.h"

static prime_ops prime_operations = {0, NULL, NULL, NULL, NULL, NULL, NULL, NULL};

static row_ops row_operations = {NULL, NULL, NULL, NULL, NULL, NULL};

#define LAZY_GREASE 1

void clean(unsigned int **m1, unsigned int d1,
           unsigned int **m2, unsigned int d2, int *map,
           unsigned int **m1_e, unsigned int **m2_e, int record,
           unsigned int grease_level, unsigned int prime,
           unsigned int len, unsigned int nob,
           unsigned int start, unsigned int start_e,
           unsigned int len_e, const char *name)
{
  unsigned int i = 0, inc = grease_level;
  grease_struct grease, grease_e;
  NOT_USED(name);
  assert(NULL != m1);
  assert(NULL != m2);
  assert(NULL != map);
  assert(NULL != name);
  assert(0 != d1);
  assert(0 != d2);
  assert(0 != prime);
  assert(0 != nob);
  assert(0 != len);
  assert(0 != grease_level);
  if (0 != record) {
    assert(NULL != m1_e);
    assert(NULL != m2_e);
  }
  grease.level = grease_level;
  grease_e.level = grease_level;
  primes_init(prime, &prime_operations);
  rows_init(prime, &row_operations);
  grease_init(&row_operations, &grease);
  grease_init(&row_operations, &grease_e);
  grease_allocate(prime, len, &grease, start);
  if (0 != record) {
    grease_allocate(prime, len_e, &grease_e, start_e);
  }
  while (i < d1) {
    unsigned int count; /* Actual amount we'll grease */
    unsigned int j = 0, k = 1;
    count = 0;
    grease_init_rows(&grease, prime);
    if (0 != record) {
      grease_init_rows(&grease_e, prime);
    }
    /* Now insert the initial elements into grease.rows */
    while (count < grease_level && i + j < d1) {
      assert(NULL != m1[i + j]);
      if (map[i + j] >= 0) {
        /* Only insert if a useful row */
        grease.rows[k - 1] = m1[i + j];
        if (0 != record) {
          grease_e.rows[k - 1] = m1_e[i + j];
        }
        k *= prime;
        count++;
      }
      j++;
    }
#if LAZY_GREASE
#else
    /* Now compute grease */
    grease_make_rows(&grease, count, prime, len);
    if (0 != record) {
      grease_make_rows(&grease_e, count, prime, len_e);
    }
#endif
    inc = j;
    /* Now clean m2 with m1[i, i + inc] */
    for (j = 0; j < d2; j++) {
      unsigned int elts = 0, l = 0;
      assert(NULL != m2[j]);
      for (k = 0; k < inc; k++) {
        if (map[i + k] >= 0) {
          elts |= (get_element_from_row(nob, map[i + k], m2[j])) << (nob * l);
          l++;
        }
      }
      if (0 != elts) {
        elts = negate_elements(elts, nob, prime);
#if LAZY_GREASE
        grease_row_inc(&grease, len, m2[j], prime, elements_contract(elts, prime, nob));
#else
        (*row_operations.incer)(grease.rows[elements_contract(elts, prime, nob) - 1], m2[j], len);
#endif
        if (0 != record) {
#if LAZY_GREASE
          grease_row_inc(&grease_e, len_e, m2_e[j], prime, elements_contract(elts, prime, nob));
#else
          (*row_operations.incer)(grease_e.rows[elements_contract(elts, prime, nob) - 1], m2_e[j], len_e);
#endif
        }
      }
    } /* for */
    i += inc;
  } /* i */
  grease_free(&grease);
  if (0 != record) {
    grease_free(&grease_e);
  }
}

void echelise(unsigned int **m, unsigned int d,
              unsigned int *d_out, int **map,
              unsigned int **m_e, int record,
              unsigned int grease_level, unsigned int prime,
              unsigned int len, unsigned int nob,
              unsigned int start, unsigned int start_e,
              unsigned int len_e,
              int full, const char *name)
{
  unsigned int dout = 0;
  unsigned int i = 0, j = 0, inc = grease_level;
  int *bits; /* The map for m */
  assert(NULL != m);
  assert(NULL != d_out);
  assert(NULL != map);
  assert(0 != d);
  assert(0 != prime);
  assert(0 != nob);
  assert(0 != len);
  assert(0 != grease_level);
  primes_init(prime, &prime_operations);
  rows_init(prime, &row_operations);
  bits = my_malloc(d * sizeof(int));
  /* i counts through m. j counts dimension */
  while (i < d) {
    unsigned int k = 0, l = 0, n;
    while (k < inc && i + l < d) {
      /* Forward clean new row with all rows */
      for (n = 0; n < l; n++) {
        unsigned int elt;
        assert(NULL != m[i + l]);
        if (bits[i + n] >= 0) {
          elt = get_element_from_row(nob, bits[i + n], m[i + l]);
          if (0 != elt) {
            elt = (*prime_operations.negate)(elt);
            if (1 == elt) {
              (*row_operations.incer)(m[i + n], m[i + l], len);
              if (0 != record) {
                (*row_operations.incer)(m_e[i + n], m_e[i + l], len_e);
              }
            } else {
              (*row_operations.scaled_incer)(m[i + n], m[i + l], len, elt);
              if (0 != record) {
                (*row_operations.scaled_incer)(m_e[i + n], m_e[i + l], len_e, elt);
              }
            }
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
          (*row_operations.scaler_in_place)(m[i + l], len, elt);
          if (0 != record) {
            (*row_operations.scaler_in_place)(m_e[i + l], len_e, elt);
          }
        }
        bits[i + l] = pos;
        k++;
      } else {
        /* This row not significant */
        bits[i + l] = -1;
      }
      l++; /* Next input row */
    } /* while */
    /* Now back clean m[i, i + l] with itself */
    for (n = 1; n < l; n++) {
      if (bits[i + n] >= 0) {
        unsigned int r;
        for (r = 0; r < n; r++) {
          unsigned int elt = get_element_from_row(nob, bits[i + n], m[i + r]);
          if (0 != elt) {
            elt = (*prime_operations.negate)(elt);
            if (1 == elt) {
              (*row_operations.incer)(m[i + n], m[i + r], len);
              if (0 != record) {
                (*row_operations.incer)(m_e[i + n], m_e[i + r], len_e);
              }
            } else {
              (*row_operations.scaled_incer)(m[i + n], m[i + r], len, elt);
              if (0 != record) {
                (*row_operations.scaled_incer)(m_e[i + n], m_e[i + r], len_e, elt);
              }
            }
          }
        }
      }
    }
    if (0 != k) {
      if (0 != full) {
        /* Only back clean for full echelise */
        /* Now clean m[0, i] with m[i, i + l] */
        if (0 != i) {
          clean(m + i, l, m, i, bits + i,
                m_e + i, m_e, record, k, prime, len,
                nob, start, start_e, len_e, name);
        }
      }
      /* Now clean m[i + l, d] with m[i, i + l] */
      if (d > i + l) {
        clean(m + i, l, m + i + l, d - i - l, bits + i,
              m_e + i, m_e + i + l, record, k, prime, len,
              nob, start, start_e, len_e, name);
      }
    }
    i += l;
    j += k;
    dout += k;
  } /* while */
  *d_out = dout;
  *map = bits;
}

unsigned int simple_echelise(unsigned int **m, unsigned int d,
                             unsigned int prime,
                             unsigned int len, unsigned int nob)
{
  unsigned int rank = 0, i = 0;
  assert(NULL != m);
  assert(0 != d);
  assert(0 != prime);
  assert(is_a_prime_power(prime));
  assert(0 != len);
  assert(0 != nob);
  primes_init(prime, &prime_operations);
  rows_init(prime, &row_operations);
  while (i < d) {
    assert(NULL != m[i]);
    if (0 == row_is_zero(m[i], len)) {
      unsigned int j, k = i + 1, elt;
      rank ++;
      elt = first_non_zero(m[i], nob, len, &j);
      if (1 != elt) {
        elt = (*prime_operations.invert)(elt);
        (*row_operations.scaler_in_place)(m[i], len, elt);
      }
      while (k < d) {
        elt = get_element_from_row(nob, j, m[k]);
        if (0 != elt) {
          elt = (*prime_operations.negate)(elt);
          if (1 == elt) {
            (*row_operations.incer)(m[i], m[k], len);
          } else {
            (*row_operations.scaled_incer)(m[i], m[k], len, elt);
          }
        }
        k++;
      } /* while k */
    } /* if */
    i++;
  } /* while i */
  return rank;
}

unsigned int simple_echelise_and_record(unsigned int **m1, unsigned int **m2,
                                        unsigned int d, unsigned int prime,
                                        unsigned int len, unsigned int nob)
{
  unsigned int rank = 0, i = 0;
  assert(NULL != m1);
  assert(NULL != m2);
  assert(0 != d);
  assert(0 != prime);
  assert(is_a_prime_power(prime));
  assert(0 != len);
  assert(0 != nob);
  primes_init(prime, &prime_operations);
  rows_init(prime, &row_operations);
  while (i < d) {
    assert(NULL != m1[i]);
    assert(NULL != m2[i]);
    if (0 == row_is_zero(m1[i], len)) {
      unsigned int j, k = i + 1, elt;
      rank ++;
      elt = first_non_zero(m1[i], nob, len, &j);
      if (1 != elt) {
        elt = (*prime_operations.invert)(elt);
        (*row_operations.scaler_in_place)(m1[i], len, elt);
        (*row_operations.scaler_in_place)(m2[i], len, elt);
      }
      while (k < d) {
        elt = get_element_from_row(nob, j, m1[k]);
        if (0 != elt) {
          elt = (*prime_operations.negate)(elt);
          if (1 == elt) {
            (*row_operations.incer)(m1[i], m1[k], len);
            (*row_operations.incer)(m2[i], m2[k], len);
          } else {
            (*row_operations.scaled_incer)(m1[i], m1[k], len, elt);
            (*row_operations.scaled_incer)(m2[i], m1[k], len, elt);
          }
        }
        k++;
      } /* while k */
    } /* if */
    i++;
  } /* while i */
  return rank;
}
