/*
 * $Id: elements.c,v 1.1 2001/08/28 21:39:44 jon Exp $
 *
 * Element manipulation for meataxe
 *
 */

#include <stdio.h>
#include <assert.h>
#include <ctype.h>
#include "utils.h"
#include "primes.h"
#include "elements.h"

int get_element(FILE *fp, unsigned int nob,
                unsigned int prime, unsigned int *el)
{
  unsigned int e = 0;

  assert(NULL != fp);
  assert(0 != nob);
  assert(0 != prime);
  while (0 == feof(fp) && 0 != nob) {
    int i = fgetc(fp);
    if (i < 0)
      return 0; /* Off end of file */
    if (isspace(i))
      continue; /* Ignore formatting */
    if (isdigit(i)) {
      e = e * 10 + (i - '0');
      nob--;
    } else {
      return 0; /* Failed, non-digit */
    }
  }
  /* Now we have e, convert to representation required by prime */
  if (0 == prime_rep(&e, prime))
    return 0; /* Failed to convert */
  *el = e;
  return 1;
}

