/*
 * $Id: primes.c,v 1.1 2001/08/28 21:39:44 jon Exp $
 *
 * Prime manipulation for meataxe
 *
 */

#include <stdio.h>
#include <assert.h>
#include "utils.h"
#include "primes.h"

int prime_rep(unsigned int *e, unsigned int prime)
{
  assert(NULL != e);
  NOT_USED(e);
  if (2 != prime)
    return 0; /* Can't handle any other primes yet */
  return 1;  
}
