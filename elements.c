/*
 * $Id: elements.c,v 1.9 2001/09/20 00:00:16 jon Exp $
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

int get_element_from_text(FILE *fp, unsigned int nod,
                          unsigned int prime, unsigned int *el)
{
  unsigned int e = 0;

  assert(NULL != fp);
  assert(0 != nod);
  assert(0 != prime);
  while (0 == feof(fp) && 0 != nod) {
    int i = fgetc(fp);
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
  assert(bits <= bits_in_unsigned_int);
  elts_per_word = bits_in_unsigned_int / nob;
  *word_offset = from / elts_per_word;
  *bit_offset = (from % elts_per_word) * nob;
  *mask = (1 << bits) - 1;
}

/* This allows cross word access */
unsigned int get_elements_from_row(const unsigned int *row, unsigned int width,
                                   unsigned int bit_offset, unsigned int mask)
{
  unsigned int word, word1;
  assert(NULL != row);
  word = *row;
  word1 = (word >> bit_offset) & mask;
  if (bit_offset + width <= bits_in_unsigned_int) {
    return word1;
  } else {
    word = (row[1] << (bits_in_unsigned_int - bit_offset)) & mask;
    return word1 | word;
  }
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

/* Convert bit field representation to power series */
unsigned int elements_contract(unsigned int elts, unsigned int prime, unsigned int nob)
{
  if (0 == prime % 2) {
    return elts; /* Exact representation for powers of 2 */
  } else {
    unsigned int out = 0, power = 1, mask = (1 << nob) - 1;
    assert(0 != nob);
    assert(0 != prime);
    assert(prime <= mask);
    while (0 != elts) {
      unsigned int elt = elts & mask;
      out += power * elt;
      elts >>= nob;
      power *= prime;
    }
    return out;
  }
}

