/*
 * $Id: utils.c,v 1.7 2001/09/30 21:49:18 jon Exp $
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

static long hadcr = 0;

/******  subroutine to get an integer like FORTRAN does  */
unsigned int getin(FILE *f, unsigned int a)
{
  int c;
  unsigned long i,j=0;
 
  if(hadcr == 1) return j;
  for(i = 0;i < a; i++) {
    c = fgetc(f);
    if(c == '\n') {
      hadcr = 1;
      return j;
    }
    if(c < '0') c = '0';
    if(c > '9') c = '0';
    j = 10*j + (c-'0');
  }
  return j;
}
 
void nextline(FILE *f)
{
  if(hadcr == 1) {
    hadcr=0;
    return;
  }
  while (fgetc(f) != '\n');
}


const char *get_str(FILE *f, char **name, unsigned int depth)
{
  int c = fgetc(f);
  if (c < 0 || my_isspace(c)) {
    if ('\n' == c) hadcr = 1;
    *name = my_malloc(depth + 1);
    (*name)[depth] = '\0';
    return *name;
  } else {
    get_str(f, name, depth + 1);
    (*name)[depth] = c;
    return *name;
  }
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
