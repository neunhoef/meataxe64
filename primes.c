/*
 * $Id: primes.c,v 1.7 2001/11/07 22:35:27 jon Exp $
 *
 * Prime manipulation for meataxe
 *
 */

#include "primes.h"
#include <stdio.h>
#include <assert.h>
#include <limits.h>
#include "utils.h"

static int prime_rep_2(unsigned int *e)
{
  NOT_USED(e);
  /* Prime and decimal rep for p = 2 are the same */
  return 1;  
}

static int decimal_rep_2(unsigned int *e)
{
  NOT_USED(e);
  /* Prime and decimal rep for p = 2 are the same */
  return 1;  
}

static unsigned int invert_2(unsigned int elt)
{
  assert(1 == elt);
  return elt;
}

static unsigned int negate_3(unsigned int elt)
{
  assert(3 > elt);
  return (0 == elt) ? elt : 3 - elt;
}

static unsigned int invert_3(unsigned int elt)
{
  assert(3 > elt && 0 != elt);
  return elt;
}

static unsigned int negate_5(unsigned int elt)
{
  assert(5 > elt);
  return (0 == elt) ? elt : 5 - elt;
}

static unsigned int invert_5(unsigned int elt)
{
  assert(5 > elt && 0 != elt);
  return (2 == elt) ? 3 : (3 == elt) ? 2 : elt;
}

static unsigned int negate_2_power(unsigned int elt)
{
  return elt;
}

static unsigned int invert_4(unsigned int elt)
{
  assert(4 > elt && 0 != elt);
  return (3 == elt) ? 2 : (2 == elt) ? 3 : elt;
}

int is_a_prime(unsigned int p)
{
  unsigned int i = 2;
  while (i*i <= p) {
    if (p % i == 0) {
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
  while (0 == is_a_prime(j)) {
    if (UINT_MAX > j)
      j++;
    else
      return 0;
  }
  *i = j;
  return 1;
}

int is_a_prime_power(unsigned int q)
{
  unsigned int i = 2;
  while (i <= q) {
    if (q % i ==  0) {
      /* Now found the prime */      
        while (q % i ==  0) {
          q /= i;
        }
        return (1 == q);
    } else {
      if (0 == next_prime(&i))
        return 0;
    }
  }
  return 0;
}

int primes_init(unsigned int prime, prime_opsp ops)
{
  if (2 == prime || 3 == prime || 4 == prime || 5 == prime) {
    /* p = 2, p = 3, p = 4, p = 5 behave the same */
    ops->prime_rep = &prime_rep_2;
    ops->decimal_rep = &decimal_rep_2;
    switch(prime) {
      case 2:
      case 4:
        ops->negate = &negate_2_power;
        ops->invert = (2 == prime) ? &invert_2 : &invert_4;
        break;
      case 3:
        ops->negate = &negate_3;
        ops->invert = &invert_3;
        break;
      case 5:
        ops->negate = &negate_5;
        ops->invert = &invert_5;
        break;
      default:
        assert(0);
        return 0;  
    }
    return 1;
  } else {
    return 0;
  }
}
