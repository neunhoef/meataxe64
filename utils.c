/*
 * $Id: utils.c,v 1.1 2001/08/28 21:39:44 jon Exp $
 *
 * Utils for meataxe
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <limits.h>
#include "utils.h"

static int is_prime(unsigned int j)
{
  unsigned int i = 2;
  while (i*i <= j) {
    if (j % i == 0) {
      return 0;
    }
    i++;
  }
  return 1;
}

static int next_prime(unsigned int *i)
{
  unsigned int j;

  assert(NULL != i);
  j = *i + 1;
  while (0 == is_prime(j)) {
    if (UINT_MAX > j)
      j++;
    else
      return 0;
  }
  *i = j;
  return 1;
}

int is_a_prime_power(unsigned int n)
{
  unsigned int i = 2;
  while (i <= n) {
    if (n % i ==  0) {
      /* Now found the prime */      
        while (n % i ==  0) {
          n /= i;
        }
        return (1 == n);
    } else {
      if (0 == next_prime(&i))
        return 0;
    }
  }
  return 0;
}

int alloc_matrix(unsigned int nob, unsigned int noc,
                 unsigned int nor, unsigned int **res)
{
  unsigned int *t;
  unsigned int size =
    (noc * nob + sizeof(unsigned int) * CHAR_BIT - 1) /
      (sizeof(unsigned int) * sizeof(char));
  /* Size in words rounded up to an unsigned int */
  assert(NULL != res);
  assert(0 != nob);
  assert(0 != nor);
  assert(0 != noc);
  size *= sizeof(unsigned int);
  /* Size in allocation units, rounded up */
  if (UINT_MAX / nor <= size)
    return 0; /* Asked for too much */
  t = malloc(size * nor);
  if (NULL == t)
    return 0; /* maloc can't cope */
  *res = t;
  return 1;
}
