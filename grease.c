/*
 * $Id: grease.c,v 1.18 2002/09/27 19:37:43 jon Exp $
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
#include "primes.h"
#include "rows.h"
#include "utils.h"

int grease_level(unsigned int prime, grease grease, unsigned int avail)
{
  unsigned int i = 1, j = prime;
  assert(NULL != grease);
  assert(is_a_prime_power(prime));
  assert(0 != avail);
  if (prime > avail + 1)
    return 0;
  while (j * prime <= avail + 1) {
    j *= prime;
    i++;
  }
  grease->level = i;
  return 1;
}

int grease_limit(unsigned int prime, unsigned int level,
                 unsigned int grease_rows, unsigned int total_rows)
{
  while (level > 1) {
    if (5 * grease_rows > total_rows) {
      level--;
      grease_rows /= prime;
    } else {
      break;
    }
  }
  return level;
}

void grease_init(row_opsp ops, grease grease)
{
  assert(NULL != grease);
  memcpy(&grease->row_operations, ops, sizeof(*ops));
}

int grease_allocate(unsigned int prime, unsigned int len,
                    grease grease, unsigned int start)
{
  unsigned int q_n;
  unsigned int i;
  assert(NULL != grease);
  assert(0 != grease->level);
  assert(0 != len);
  assert(0 != prime);
  if (0 == int_pow(prime, grease->level, &q_n)) {
    return 0;
  }
  grease->size = q_n - 1;
  grease->status = matrix_malloc(grease->size);
  /* Get the table pointers */
  grease->rows = matrix_malloc(grease->size);
  for (i = 0; i < grease->size; i++) {
    grease->rows[i] = memory_pointer_offset(start, i, len);
  }
  return 1;
}

void grease_init_rows(grease grease, unsigned int prime)
{
  /* Fill in grease->status */
  unsigned int i, j;
  assert(NULL != grease);
  assert(0 != grease->level);
  memset(grease->status, 0, grease->size * sizeof(unsigned int));
  j = 1;
  for (i = 0; i < grease->level; i++) {
    grease->status[j - 1] = 1; /* An entry from the matrix */
    j *= prime; /* Next entry directly from matrix */
  }
}

void grease_free(grease grease)
{
  assert(NULL != grease);
  assert(NULL != grease->rows);
  assert(NULL != grease->status);
  free(grease->status);  
  free(grease->rows);
  grease->status = NULL;
  grease->rows = NULL;
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
static void grease_make_row(grease grease, unsigned int i, unsigned int prime, unsigned int len)
{
  unsigned int j = i - 1;
  assert(0 < i);
  if (0 == grease->status[j]) {
    unsigned int quot, rem, index;
    split(prime, i, &quot, &rem, &index);
    assert(0 != grease->status[index - 1]);
    if (0 == rem) {
      /* Scaled operation, shouldn't be by 1 */
      assert(1 != quot);
      (*grease->row_operations.scaler)(grease->rows[index - 1], grease->rows[j], len, quot);
    } else {
      unsigned int k = index * quot - 1;
      if (0 == grease->status[rem - 1]) {
        grease_make_row(grease, rem, prime, len);
      }
      if (0 == grease->status[k]) {
        (*grease->row_operations.scaler)(grease->rows[index - 1], grease->rows[k], len, quot);
        grease->status[k] = 1;
      }
      (*grease->row_operations.adder)(grease->rows[rem - 1], grease->rows[k], grease->rows[j], len);
      grease->status[j] = 1;
    }
  }
}

#if 0
void grease_make_rows(grease grease, unsigned int level,
                      unsigned int prime, unsigned int len)
{
  unsigned int i;
  unsigned int table_size = grease->size;
  assert(0 != table_size);
  assert(0 != len);
  assert(0 != prime);
  assert(NULL != grease);
  if (level < grease->level) {
    unsigned int q_n;
    if (0 == pow(prime, level, &q_n)) {
      assert(0 != pow(prime, level, &q_n));
    }
    table_size = q_n - 1;
  }
  /* Compute non-fixed rows */
  for (i = 0; i < table_size; i++) {
    if (0 == grease->status[i]) {
      grease_make_row(grease, i + 1, prime, len);
    }
  }
}
#endif

void grease_row_inc(grease grease, unsigned int len, unsigned int *row,
                    unsigned int prime, unsigned int element)
{
  assert(NULL != row);
  assert(NULL != grease);
  assert(element >= 1);
  if (0 == grease->status[element - 1]) {
    grease_make_row(grease, element, prime, len);
  }
  (*grease->row_operations.incer)(grease->rows[element - 1], row, len);
}
