/*
 * $Id: primes.c,v 1.3 2001/09/05 22:47:25 jon Exp $
 *
 * Prime manipulation for meataxe
 *
 */

#include <stdio.h>
#include <assert.h>
#include "utils.h"
#include "primes.h"

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

int primes_init(unsigned int prime, prime_opsp ops)
{
  if (2 == prime || 3 == prime) {
    /* p = 2 and p = 3 behave the same */
    ops->prime_rep = &prime_rep_2;
    ops->decimal_rep = &decimal_rep_2;
    return 1;
  } else {
    return 0;
  }
}
