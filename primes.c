/*
 * $Id: primes.c,v 1.11 2002/06/25 10:30:12 jon Exp $
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

static unsigned int power_2(unsigned int elt, unsigned int n)
{
  NOT_USED(n);
  return elt;
}

static unsigned int add_2(unsigned int elt1, unsigned int elt2)
{
  assert(2 > elt1 && 2 > elt2);
  return elt1 ^ elt2;
}

static unsigned int add_4(unsigned int elt1, unsigned int elt2)
{
  assert(4 > elt1 && 4 > elt2);
  return elt1 ^ elt2;
}

static unsigned int mul_2(unsigned int elt1, unsigned int elt2)
{
  assert(2 > elt1 && 2 > elt2);
  return elt1 * elt2;
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

static unsigned int power_3(unsigned int elt, unsigned int n)
{
  assert(3 > elt);
  NOT_USED(n);
  return elt;
}

static unsigned int add_3(unsigned int elt1, unsigned int elt2)
{
  assert(3 > elt1 && 3 > elt2);
  return (elt1 + elt2) % 3;
}

static unsigned int mul_3(unsigned int elt1, unsigned int elt2)
{
  assert(3 > elt1 && 3 > elt2);
  return (elt1 * elt2) % 3;
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

static unsigned int power_5(unsigned int elt, unsigned int n)
{
  assert(5 > elt);
  NOT_USED(n);
  return elt;
}

static unsigned int add_5(unsigned int elt1, unsigned int elt2)
{
  assert(5 > elt1 && 5 > elt2);
  return (elt1 + elt2) % 5;
}

static unsigned int mul_5(unsigned int elt1, unsigned int elt2)
{
  assert(5 > elt1 && 5 > elt2);
  return (elt1 * elt2) % 5;
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

static unsigned int power_4(unsigned int elt, unsigned int n)
{
  assert(4 > elt);
  n = n % 4;
  assert(2 == n || 0 == n);
  return (0 == n || 1 >= elt) ? elt : (2 == elt) ? 3 : 2;
}

static unsigned int mul_4(unsigned int elt1, unsigned int elt2)
{
  assert(4 > elt1 && 4 > elt2);
  return (0 == elt1 || 0 == elt2) ? 0 :
    (1 == elt1) ? elt2 :
    (1 == elt2) ? elt1 : 
    (elt1 != elt2) ? 1 : 5 - elt1;
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
  unsigned int i = prime_divisor(q);
  if (0 != i) {
    while (q % i ==  0) {
      q /= i;
    }
    return (1 == q);
  }
  return 0;
}

unsigned int prime_divisor(unsigned int q)
{
  unsigned int i = 2;
  while (i <= q) {
    if (q % i ==  0) {
      return i;
    } else {
      if (0 == next_prime(&i)) {
        return 0;
      }
    }
  }
  return 0;
}

static unsigned int prime_index_aux(unsigned int i, unsigned int q, unsigned int prime)
{
  if (1 == q) {
    return i;
  } else {
    assert(0 == q % prime);
    return prime_index_aux(i + 1, q / prime, prime);
  }
}

unsigned int prime_index(unsigned int q, unsigned int prime)
{
  assert(0 != prime);
  assert(0 != q);
  assert(0 == q % prime);
  return prime_index_aux(0, q, prime);
}

int primes_init(unsigned int prime, prime_opsp ops)
{
  if (2 == prime || 3 == prime || 4 == prime || 5 == prime) {
    /* p = 2, p = 3, p = 4, p = 5 behave the same */
    ops->prime = prime;
    ops->prime_rep = &prime_rep_2;
    ops->decimal_rep = &decimal_rep_2;
    switch(prime) {
      case 2:
      case 4:
        ops->negate = &negate_2_power;
        ops->invert = (2 == prime) ? &invert_2 : &invert_4;
        ops->power = (2 == prime) ? &power_2 : &power_4;
        ops->add = (2 == prime) ? &add_2 : add_4;
        ops->mul = (2 == prime) ? &mul_2 : &mul_4;
        break;
      case 3:
        ops->negate = &negate_3;
        ops->invert = &invert_3;
        ops->power = power_3;
        ops->add = &add_3;
        ops->mul = &mul_3;
        break;
      case 5:
        ops->negate = &negate_5;
        ops->invert = &invert_5;
        ops->power = power_5;
        ops->add = &add_5;
        ops->mul = &mul_5;
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
