/*
 * $Id: grease.c,v 1.5 2001/09/18 23:15:46 jon Exp $
 *
 * Functions to grease matrix rows
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <limits.h>
#include "elements.h"
#include "matrix.h"
#include "memory.h"
#include "rows.h"
#include "utils.h"
#include "grease.h"

static unsigned grease_table_size;
static unsigned grease_table_rows;
static unsigned int **grease_table;
/* An array giving the status of rows in the grease table */
/* 0 => unset, 1 => set */
static int *grease_table_comp;
static row_ops row_operations = { NULL, NULL, NULL};

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

int grease(unsigned int nob, unsigned int prime, unsigned int *step, unsigned int avail)
{
  unsigned int i = 1, j = prime;
  unsigned int elts_per_word = bits_in_unsigned_int / nob;
  assert(NULL != step);
  if (2 != prime) {
    *step = 1;
    return 1;
  }
  while (j * prime <= avail + 1) {
    j *= prime;
    i++;
  }
  while (0 != elts_per_word % i) {
    i--;
  }
  printf("Using grease level %d\n", i);
  *step = i;
  return 1;
}

int grease_allocate_rows(unsigned int size,
                         unsigned int prime, unsigned int len,
                         unsigned int ***rows_out,
                         unsigned int start)
{
  unsigned int q_n;
  assert(0 != size);
  assert(0 != len);
  assert(0 != prime);
  if (1 == size) {
    *rows_out = NULL;
    return 1;
  } else {
    unsigned int i, j;
    void *temp;
    if (0 == pow(prime, size, &q_n)) {
      return 0;
    }
    grease_table_rows = size;
    grease_table_size = q_n - 1;
    if (0 == matrix_malloc(grease_table_size, &temp)) {
      return 0;
    }
    grease_table_comp = temp;
    /* Fill in grease_table_comp */
    memset(grease_table_comp, 0, grease_table_size * sizeof(unsigned int));
    j = 1;
    for (i = 0; i < size; i++) {
      grease_table_comp[(j) - 1] = 1; /* An entry from the matrix */
      j *= prime; /* Next entry directly from matrix */
    }
    /* Get the table pointers */
    if (0 == matrix_malloc(grease_table_size, &temp)) {
      matrix_free(grease_table_comp);
      return 0;
    }
    grease_table = temp;
    for (i = 0; i < grease_table_size; i++) {
      grease_table[i] = memory_pointer_offset(start, i, len);
    }
    *rows_out = grease_table;
    return 1;
  }
}

void grease_free_rows(unsigned int **rows)
{
  assert(NULL != rows);
  free(grease_table_comp);  
  free(rows);
}

int grease_make_rows(unsigned int size,
                     unsigned int prime, unsigned int len)
{
  assert(0 != size);
  assert(0 != len);
  assert(0 != prime);
  if (1 == size) {
    /* Already in the right place */
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
    /* Compute non-fixed rows */
    k = 0;
    for (i = 0; i < table_size; i++) {
      if (0 == grease_table_comp[i]) {
        assert(2 <= i);
        /* Note, p = 2 specific code follows */
        /* Rewrite i+1 as k + j+1, where j+1 < k, and k is a power of 2 */
        /* Then add */
        j = i - k; /* (i + 1) - k - 1 */
        if (0 == (*adder)(grease_table[k - 1], grease_table[j], grease_table[i],
                          len)) {
          /* Deallocate all and return 0 */
          return 0;
        }
      } else {
        k = (0 == k) ? 1 : k << 1;
        /* k refers to the most recent copied row */
      }
    }
    return 1;
  }
}

void grease_init(row_opsp ops)
{
  memcpy(&row_operations, ops, sizeof(row_operations));
}
