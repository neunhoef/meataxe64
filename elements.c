/*
 * $Id: elements.c,v 1.2 2001/08/30 18:31:45 jon Exp $
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

int get_element_from_text(const FILE *fp, unsigned int nob,
                          unsigned int prime, unsigned int *el)
{
  unsigned int e = 0;

  assert(NULL != fp);
  assert(0 != nob);
  assert(0 != prime);
  while (0 == feof((FILE *)fp) && 0 != nob) {
    int i = fgetc((FILE *)fp);
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

unsigned int get_element_from_row(unsigned int nob, unsigned int index,
                                  const unsigned int *row)
{
  unsigned int elts_per_word = bits_in_unsigned_int / nob;
  unsigned int word_offset = index / elts_per_word;
  unsigned int bit_offset = index % elts_per_word;
  unsigned int word = row[word_offset];
  unsigned int base_mask = (1 << nob) - 1;
  unsigned int mask = base_mask << (bit_offset * nob);
  unsigned int res = (word & mask) >> (bit_offset * nob);
  assert(0 != nob);
  assert(NULL != row);
  return res;
}

