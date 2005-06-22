/*
 * $Id: grease.c,v 1.26 2005/06/22 21:52:53 jon Exp $
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
#include "parse.h"
#include "primes.h"
#include "rows.h"
#include "utils.h"

int grease_level(u32 prime, grease grease, u32 avail)
{
  u32 i = 1, j = prime;
  assert(NULL != grease);
  assert(is_a_prime_power(prime));
  assert(0 != avail);
  if (prime > avail + 1)
    return 0;
  while (j * prime <= avail + 1 && i < max_grease) {
    j *= prime;
    i++;
  }
  grease->level = i;
  grease->size = j - 1;
  return 1;
}

int grease_limit(u32 prime, u32 level,
                 u32 grease_rows, u32 total_rows)
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

/* For an index into the table, determine the split as a sum of vector */
/* quot * V[index] + V[rem] */
static void split(u32 prime, u32 n,
                  u32 *quot, u32 *rem, u32 *index)
{
  u32 power, div;
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

int grease_allocate(u32 prime, u32 len,
                    grease grease, u32 start)
{
  u32 q_n;
  u32 i;
  assert(NULL != grease);
  assert(0 != grease->level);
  assert(0 != len);
  assert(0 != prime);
  if (0 == int_pow(prime, grease->level, &q_n)) {
    return 0;
  }
  grease->size = q_n - 1;
  grease->status = matrix_malloc(grease->size);
  grease->quot = matrix_malloc(grease->size);
  grease->rem = matrix_malloc(grease->size);
  grease->index = matrix_malloc(grease->size);
  /* Get the table pointers */
  grease->rows = matrix_malloc(grease->size);
  for (i = 0; i < grease->size; i++) {
    grease->rows[i] = memory_pointer_offset(start, i, len);
    split(prime, i + 1, grease->quot + i, grease->rem + i, grease->index + i);
  }
  return 1;
}

void grease_init_rows(grease grease, u32 prime)
{
  /* Fill in grease->status */
  u32 i, j, k, *l;
  assert(NULL != grease);
  assert(0 != grease->level);
  j = grease->size;
  for (i = 0; i < j; i++) {
    grease->status[i] = 0;
  }
  j = 1;
  k = grease->level;
  l = grease->status - 1;
  for (i = 0; i < k; i++) {
    l[j] = 1; /* An entry from the matrix */
    j *= prime; /* Next entry directly from matrix */
  }
}

void grease_free(grease grease)
{
  assert(NULL != grease);
  assert(NULL != grease->rows);
  assert(NULL != grease->status);
  free(grease->status);  
  free(grease->quot);
  free(grease->rem);
  free(grease->index);
  free(grease->rows);
  grease->status = NULL;
  grease->rows = NULL;
}

/* Compute, if necessary, grease row i */
/* i is uncorrected for the table, so should not be zero */
/* len is the revised length taking offset into account */
static void grease_make_row(grease grease, u32 i, u32 len, u32 offset)
{
  u32 j = i - 1;
  u32 quot, rem, index;
  assert(0 < i);
  assert(0 == grease->status[j]);
  quot = grease->quot[j];
  rem = grease->rem[j];
  index = grease->index[j];
  assert(0 != grease->status[index - 1]);
  if (0 == rem) {
    /* Scaled operation, shouldn't be by 1 */
    assert(1 != quot);
    (*grease->row_operations.scaler)(grease->rows[index - 1] + offset, grease->rows[j] + offset, len, quot);
  } else {
    u32 k = index * quot - 1;
    if (0 == grease->status[rem - 1]) {
      grease_make_row(grease, rem, len, offset);
    }
    if (0 == grease->status[k]) {
      (*grease->row_operations.scaler)(grease->rows[index - 1] + offset, grease->rows[k] + offset, len, quot);
      grease->status[k] = 1;
    }
    (*grease->row_operations.adder)(grease->rows[rem - 1] + offset, grease->rows[k] + offset,
                                    grease->rows[j] + offset, len);
  }
  grease->status[j] = 1;
}

void grease_row_inc(grease grease, u32 len, word *row,
                    u32 element, u32 offset)
{
  u32 l = len - offset;
  u32 e = element - 1;
  assert(NULL != row);
  assert(NULL != grease);
  assert(element >= 1);
  assert(len >= offset);
  if (0 == grease->status[e]) {
    grease_make_row(grease, element, l, offset);
  }
  (*grease->row_operations.incer)(grease->rows[e] + offset, row + offset, l);
}
