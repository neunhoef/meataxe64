/*
 * $Id: utils.c,v 1.4 2001/09/04 23:00:12 jon Exp $
 *
 * Utils for meataxe
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <limits.h>
#include <ctype.h>
#include "utils.h"

unsigned int bits_in_unsigned_int = CHAR_BIT * sizeof(unsigned int);

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

int read_decimal(const char *str, unsigned int len, unsigned int *out)
{
  unsigned int res = 0;

  assert(0 < len);
  assert(len <= 6);
  while (isspace(*str) && len > 0) {
    len--;
    str++;
  }
  while (len > 0) {
    if (isdigit(*str)) {
      res = res*10 + (*str-'0');
      str++;
      len--;
    } else {
      fprintf(stderr, "Unrecognised digit '%c'\n", *str);
      return 0;
    }
  }
  *out = res;
  return 1;
}

unsigned int bits_of(unsigned int n)
{
  unsigned int i = 0;
  assert(n >= 1);
  n -= 1;
  while(n > 0) {
    n >>= 1;
    i++;
  }
  return i;
}

/* Compute the number of decimal digits needed to represent 0 - n-1 */
unsigned int digits_of(unsigned int n)
{
  unsigned int i = 0;
  assert(n >= 1);
  n -= 1;
  while(n > 0) {
    n /= 10;
    i++;
  }
  return i;
}
