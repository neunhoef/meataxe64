/*
 * $Id: grease.c,v 1.9 2001/11/12 13:43:38 jon Exp $
 *
 * Functions to grease matrix rows
 *
 */

#include "grease.h"
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

static unsigned grease_table_size;
static unsigned grease_table_rows;
static unsigned int **grease_table;
/* An array giving the status of rows in the grease table */
/* 0 => unset, 1 => set */
static int *grease_table_comp;
static row_ops row_operations = { NULL, NULL, NULL, NULL};

int grease(unsigned int prime, unsigned int *step, unsigned int avail)
{
  unsigned int i = 1, j = prime;
  assert(NULL != step);
  if (prime > avail + 1)
    return 0;
  while (j * prime <= avail + 1) {
    j *= prime;
    i++;
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
  unsigned int i;
  void *temp;
  assert(0 != size);
  assert(0 != len);
  assert(0 != prime);
  if (0 == pow(prime, size, &q_n)) {
    return 0;
  }
  grease_table_rows = size;
  grease_table_size = q_n - 1;
  if (0 == matrix_malloc(grease_table_size, &temp)) {
    return 0;
  }
  grease_table_comp = temp;
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

void grease_init_rows(unsigned int size, unsigned int prime)
{
  /* Fill in grease_table_comp */
  unsigned int i, j;
  memset(grease_table_comp, 0, grease_table_size * sizeof(unsigned int));
  j = 1;
  for (i = 0; i < size; i++) {
    grease_table_comp[j - 1] = 1; /* An entry from the matrix */
    j *= prime; /* Next entry directly from matrix */
  }
}

void grease_free_rows(unsigned int **rows)
{
  assert(NULL != rows);
  free(grease_table_comp);  
  free(rows);
}

/* For an index into the table, determine the split as a sum of vector */
/* quot * V[index] + V[rem] */
static void split(unsigned int prime, unsigned int n,
                  unsigned int *quot, unsigned int *rem, unsigned int *index)
{
  unsigned int power, div;
  assert(0 != n);
  assert(0 != prime);
  assert(NULL != quot);
  assert(NULL != rem);
  assert(NULL != index);
  if (2 == prime) {
    div = n >> 1;
    power = 1;
    while (0 != div) {
      div >>= 1;
      power <<= 1;
    }
    *quot = n / power;
    *index = power;
    *rem = n & (power - 1);
  } else {
    power = prime;
    while (n >= power) {
      div = n / power;
      if (div < prime) {
        assert(0 != div);
        *index = power;
        *quot = div;
        *rem = n % power;
        return;
      }
      power *= prime;
    }
    *quot = n;
    *index = 1;
    *rem = 0;
  }
}

/* Compute, if necessary, grease row i */
/* i is uncorrected for the table, so should not be zero */
static int grease_make_row(unsigned int i, unsigned int prime, unsigned int len)
{
  unsigned int j = i - 1;
  assert(0 < i);
  if (0 != grease_table_comp[j]) {
    return 1; /* Already done. This should catch scale by 1 */
  } else {
    unsigned int quot, rem, index;
    split(prime, i, &quot, &rem, &index);
    assert(0 != grease_table_comp[index - 1]);
    if (0 == rem) {
      /* Scaled operation, shouldn't be by 1 */
      assert(1 != quot);
      (*row_operations.scaler)(grease_table[index - 1], grease_table[j], len, quot);
      return 1;
    } else {
      int ok = (0 != grease_table_comp[rem - 1]) || grease_make_row(rem, prime, len);
      unsigned int k = index * quot - 1;
      if (ok) {
        if (0 == grease_table_comp[k]) {
          (*row_operations.scaler)(grease_table[index - 1], grease_table[k], len, quot);
          grease_table_comp[k] = 1;
        }
        (*row_operations.adder)(grease_table[rem - 1], grease_table[k], grease_table[j], len);
        grease_table_comp[j] = 1;
      }
      return ok;
    }
  }
}

int grease_make_rows(unsigned int size,
                     unsigned int prime, unsigned int len)
{
  unsigned int i;
  unsigned int table_size = grease_table_size;
  assert(0 != size);
  assert(0 != len);
  assert(0 != prime);
  if (size < grease_table_rows) {
    unsigned int q_n;
    if (0 == pow(prime, size, &q_n)) {
      assert(0 != pow(prime, size, &q_n));
    }
    table_size = q_n - 1;
  }
  /* Compute non-fixed rows */
  for (i = 0; i < table_size; i++) {
    if (0 == grease_table_comp[i]) {
      if (0 == grease_make_row(i + 1, prime, len)) {
        return 0;
      }
    }
  }
  return 1;
}

void grease_init(row_opsp ops)
{
  memcpy(&row_operations, ops, sizeof(row_operations));
}
