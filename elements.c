/*
 * $Id: elements.c,v 1.7 2001/09/09 22:34:11 jon Exp $
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

static prime_ops prime_operations = { NULL, NULL};
static int inited = 0;

int get_element_from_text(const FILE *fp, unsigned int nod,
                          unsigned int prime, unsigned int *el)
{
  unsigned int e = 0;

  assert(NULL != fp);
  assert(0 != nod);
  assert(0 != prime);
  while (0 == feof((FILE *)fp) && 0 != nod) {
    int i = fgetc((FILE *)fp);
    if (i < 0)
      return 0; /* Off end of file */
    if (my_isspace(i))
      continue; /* Ignore formatting */
    if (isdigit(i)) {
      e = e * 10 + (i - '0');
      nod--;
    } else {
      printf("Failed on '%c', where isspace delivered %d\n", i, isspace(i));
      return 0; /* Failed, non-digit */
    }
  }
  /* Now we have e, convert to representation required by prime */
  if (0 == inited && 0 == primes_init(prime, &prime_operations)) {
    return 0;
  }
  inited = 1;
  if (0 == (*prime_operations.prime_rep)(&e))
    return 0; /* Failed to convert */
  *el = e;
  return 1;
}

unsigned int get_element_from_row(unsigned int nob, unsigned int index,
                                  const unsigned int *row)
{
  unsigned int elts_per_word = bits_in_unsigned_int / nob;
  unsigned int word_offset = index / elts_per_word;
  unsigned int bit_offset = (index % elts_per_word) * nob;
  unsigned int word = row[word_offset];
  unsigned int mask = (1 << nob) - 1;
  unsigned int res = (word >> bit_offset) & mask;
  assert(0 != nob);
  assert(NULL != row);
  assert(bit_offset + nob <= bits_in_unsigned_int);
  return res;
}

void element_access_init(unsigned int nob, unsigned int from, unsigned int size,
                         unsigned int *word_offset, unsigned int *bit_offset,
                         unsigned int *mask)
{
  unsigned int elts_per_word;
  unsigned int bits = nob * size;
  assert(0 != nob);
  assert(0 != size);
  elts_per_word = bits_in_unsigned_int / nob;
  *word_offset = from / elts_per_word;
  *bit_offset = (from % elts_per_word) * nob;
  assert(*bit_offset + bits <= bits_in_unsigned_int);
  *mask = (1 << bits) - 1;
}

unsigned int get_elements_from_row(const unsigned int *row,
                                   unsigned int bit_offset, unsigned int mask)
{
  unsigned int word;
  assert(NULL != row);
  word = *row;
  return (word >> bit_offset) & mask;
}

void put_element_to_row(unsigned int nob, unsigned int index,
                        unsigned int *row, unsigned int elt)
{
  unsigned int elts_per_word = bits_in_unsigned_int / nob;
  unsigned int word_offset = index / elts_per_word;
  unsigned int bit_offset = (index % elts_per_word) * nob;
  unsigned int word = row[word_offset];
  unsigned int base_mask = (1 << nob) - 1;
  unsigned int mask = base_mask << bit_offset;
  assert(0 != nob);
  assert(NULL != row);
  assert(bit_offset + nob <= bits_in_unsigned_int);
  elt = elt << bit_offset;
  word = (word & (mask ^ 0xffffffff)) | elt;
  row[word_offset] = word;
}
