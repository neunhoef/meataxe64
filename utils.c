/*
 * $Id: utils.c,v 1.6 2001/09/25 22:31:58 jon Exp $
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

int my_isspace(int i)
{
    return (i == ' ') || (i == 9) || (i == 10) || (i == 13);
}

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

int read_decimal(const char *str, unsigned int len, unsigned int *out)
{
  unsigned int res = 0;

  assert(0 < len);
  assert(len <= 6);
  while (my_isspace(*str) && len > 0) {
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

void *my_malloc(size_t size)
{
  void *ret;
  assert(0 != size);
  ret = malloc(size);
  if (NULL == ret) {
    fprintf(stderr, "Failed to allocate %d bytes, terminating\n", size);
    exit(1);
  }
  return ret;
}
