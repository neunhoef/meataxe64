/*
 * $Id: grease.c,v 1.3 2001/09/08 12:40:55 jon Exp $
 *
 * Functions to grease matrix rows
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <limits.h>
#include "utils.h"
#include "elements.h"
#include "rows.h"
#include "grease.h"

static unsigned grease_table_size;
static unsigned grease_table_rows;
static unsigned int **grease_table;
static int *grease_table_alloc;
static row_ops row_operations = { NULL, NULL };

static int pow(unsigned int n, unsigned int index,
               unsigned int *res)
{
  assert(0 != n);
  if (0 == index) {
    *res = 1;
    return 1;
  } else if (1 == index) {
    *res = n;
    return 1;
  } else {
    if (0 != pow(n, index-1, res)) {
      if (*res < (UINT_MAX / n)) {
        *res *= n;
        return 1;
      } else {
        return 0;
      }
    } else {
      return 0;
    }
  }
}

#define MAX_GREASE 8
static unsigned int grease_calc(unsigned int nor, unsigned int prime,
                                unsigned int constraint, unsigned int nob)
{
  unsigned int costs[MAX_GREASE];
  unsigned int n;
  unsigned int elts_per_word = bits_in_unsigned_int / nob;
  unsigned int best = 0;
  unsigned int best_index = 1;
  for (n = 0; n < MAX_GREASE; n++) {
    unsigned int q_n;
    unsigned int Cg;
    unsigned int Cu;
    unsigned int Cng;
    unsigned int cons;
    if (0 != pow(prime, n + 1, &q_n)) {
      Cg = q_n - (prime - 1) * (n + 1) - 1; /* Cost of computing grease */
      Cu = (nor - nor / q_n)/(n + 1); /* Cost of use */
      Cng = nor - nor / prime;
      cons = nor + Cg;
      if (cons <= constraint) {
        if (Cng >= Cg + Cu) {
          costs[n] = Cng - Cg - Cu;
#if 0
          printf("Grease level %d, non-grease cost %d, grease computation cost %d, grease use cost %d, grease difference %d, grease ratio %d\n", n + 1, Cng, Cg, Cu, Cng - Cg - Cu, Cng / (Cg + Cu));
#endif
        } else {
          costs[n] = 0;
        }
      } else {
        costs[n] = 0;
      }
    } else {
      costs[n] = 0;
    }
  }
  /* Now compute the best cost */
  /* Favour higher grease levels, and divisibility into elts_per_word */
  for (n = 0; n < MAX_GREASE; n++) {
    if (costs[n] > best) {
      best = costs[n];
      best_index = n + 1;
    }
  }
  while (0 != elts_per_word % (best_index) && best_index <= MAX_GREASE &&
      0 != pow(prime, best_index, &best)) {
    /* Search for a match */
    best_index++;
#if 0
    printf("Incrementing best index to %d\n", best_index);
#endif
  }
  while ((0 == pow(prime, best_index, &best) || 0 != elts_per_word % (best_index)) &&
         best_index > 1) {
    /* Still no match, search down */
    best_index--;
  }
#if 0
  printf("Settled at best_index %d, cost %d\n", best_index, costs[best_index - 1]);
#endif
  return (nor < MAX_GREASE) ? nor : best_index;
}

int grease(unsigned int nob, unsigned int nor1, unsigned int noc1,
           unsigned int noc2, unsigned int prime, unsigned int *step)
{
  assert(NULL != step);
  NOT_USED(nor1);
  NOT_USED(noc2);
  /* Only do this for p = 2 at present */
  *step = (2 == prime) ? grease_calc(noc1, prime, UINT_MAX, nob) : 1;
  return 1;
}

int grease_allocate_rows(unsigned int size,
                         unsigned int prime, unsigned int nob, unsigned int noc,
                         unsigned int *grease_row_count,
                         unsigned int ***rows_out)
{
  unsigned int q_n;
  assert(NULL != grease_row_count);
  assert(0 != size);
  assert(0 != nob);
  assert(0 != noc);
  assert(0 != prime);
  if (0 == rows_init(prime, &row_operations)) {
    return 0;
  }
  if (1 == size) {
    *grease_row_count = 1;
    *rows_out = NULL;
    return 1;
  } else {
    unsigned int i, j;
    if (0 == pow(prime, size, &q_n)) {
      return 0;
    }
    grease_table_rows = size;
    grease_table_size = q_n - 1;
    grease_table_alloc = malloc(grease_table_size * sizeof(unsigned int));
    if (NULL == grease_table_alloc) {
      return 0;
    }
    grease_table = malloc(grease_table_size * sizeof(unsigned int));
    if (NULL == grease_table) {
      free(grease_table_alloc);
      return 0;
    }
    /* Fill in grease_table_alloc */
    memset(grease_table_alloc, 0, grease_table_size * sizeof(unsigned int));
    for (i = 0; i < size; i++) {
      grease_table_alloc[(1 << i) - 1] = 1;
      /* Note, specific to p = 2 */
      /* General case involves (p**i), and also linear multiples */
    }
    for (i = 0; i < grease_table_size; i++) {
      if (0 == grease_table_alloc[i]) {
        if (0 == row_malloc(nob, noc, grease_table + i)) {
          free(grease_table_alloc);
          /* Free all so far allocated */
          for (j = 0; j < i; j++) {
            if (0 == grease_table_alloc[j]) {
              free(grease_table[j]);
            }
          }
          free(grease_table);
          return 0;
        }
      }
    }
    *grease_row_count = grease_table_size;
    *rows_out = grease_table;
    return 1;
  }
}

int grease_make_rows(unsigned int **rows, unsigned int size,
                     unsigned int prime, unsigned int noc,
                     unsigned int col_index,
                     unsigned int ***rows_out)
{
  assert(NULL != rows_out);
  assert(NULL != rows);
  assert(0 != size);
  assert(0 != noc);
  assert(0 != prime);
  if (1 == size) {
    *rows_out = &(rows[col_index]);
    return 1;
  } else {
    unsigned int i, j, k;
    unsigned int table_size = grease_table_size;
    row_adder adder = row_operations.adder;
    assert(NULL != adder);
    if (size < grease_table_rows) {
      unsigned int q_n;
      if (0 == pow(prime, size, &q_n)) {
        assert(0 != pow(prime, size, &q_n));
      }
      table_size = q_n - 1;
    }
    k = col_index;
    /* Point to the rows to be used */
    for (i = 0; i < table_size; i++) {
      if (0 != grease_table_alloc[i]) {
        grease_table[i] = rows[k];
        k++;
      }
    }
    /* Compute non-fixed rows */
    k = 0;
    for (i = 0; i < table_size; i++) {
      if (0 == grease_table_alloc[i]) {
        assert(2 <= i);
        /* Note, p = 2 specific code follows */
        /* Rewrite i+1 as k + j+1, where j+1 < k, and k is a power of 2 */
        /* Then add */
        j = i - k; /* (i + 1) - k - 1 */
        if (0 == (*adder)(grease_table[k - 1], grease_table[j], grease_table[i],
                          noc)) {
          /* Deallocate all and return 0 */
          return 0;
        }
      } else {
        k = (0 == k) ? 1 : k << 1;
        /* k refers to the most recent copied row */
      }
    }
    *rows_out = grease_table;
    return 1;
  }
}
