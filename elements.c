/*
 * $Id: elements.c,v 1.22 2004/05/02 19:33:19 jon Exp $
 *
 * Element manipulation for meataxe
 *
 */

#include "elements.h"
#include <stdio.h>
#include <assert.h>
#include <ctype.h>
#include "utils.h"
#include "primes.h"

static prime_ops prime_operations = {0, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
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

unsigned int get_element_from_char_row(unsigned int eperb, unsigned int prime,
                                       unsigned int index, const unsigned char *row)
{
  unsigned int char_offset;
  unsigned int field;
  unsigned int word;
  unsigned int i = 0;
  assert(0 != eperb);
  assert(NULL != row);
  assert(0 != prime);
  char_offset = index / eperb;
  field = (eperb - 1 - (index % eperb));
  word = row[char_offset];
  while (i < field) {
    word /= prime;
    i++;
  }
  return word % prime;
}

unsigned int get_mask_and_elts(unsigned int nob, unsigned int *elts_per_word)
{
  assert(NULL != elts_per_word);
  *elts_per_word = bits_in_unsigned_int / nob;
  return (1 << nob) - 1;
}

unsigned int get_element_from_row_with_params(unsigned int nob, unsigned int index, unsigned int mask,
                                              unsigned int elts_per_word, const unsigned int *row)
{
  unsigned int word_offset = index / elts_per_word;
  unsigned int bit_offset = (index % elts_per_word) * nob;
  unsigned int word = row[word_offset];
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

unsigned int get_elements_in_word_from_row(const unsigned int *row,
                                           unsigned int bit_offset, unsigned int mask)
{
  assert(NULL != row);
  return (*row >> bit_offset) & mask;
}

unsigned int get_elements_out_word_from_row(const unsigned int *row,
                                            unsigned int shift,
                                            unsigned int bit_offset, unsigned int mask)
{
  unsigned int word1, word2;
  assert(NULL != row);
  word1 = *row;
  word2 = (word1 >> bit_offset);
  word1 = (row[1] << shift) & mask;
  return word2 | word1;
}

/* This allows cross word access */
unsigned int get_elements_from_row(const unsigned int *row, unsigned int width,
                                   unsigned int nob,
                                   unsigned int bit_offset, unsigned int mask)
{
  unsigned int word1, word2;
  assert(NULL != row);
  assert(0 != width);
  word1 = *row;
  word2 = (word1 >> bit_offset) & mask;
  if (bit_offset + width <= bits_in_unsigned_int) {
    return word2;
  } else {
    unsigned int elts_in_word1 = (bits_in_unsigned_int - bit_offset) / nob;
    word1 = (row[1] << (elts_in_word1 * nob)) & mask;
    return word2 | word1;
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

void put_element_to_row_with_params(unsigned int nob, unsigned int index, unsigned int mask,
                                    unsigned int elts_per_word, unsigned int *row, unsigned int elt)
{
  unsigned int word_offset = index / elts_per_word;
  unsigned int bit_offset = (index % elts_per_word) * nob;
  unsigned int word;
  row += word_offset;
  word = *row;
  mask = mask << bit_offset;
  assert(0 != nob);
  assert(NULL != row);
  assert(bit_offset + nob <= bits_in_unsigned_int);
  elt = elt << bit_offset;
  word = (word & (~mask)) | elt;
  *row = word;
}

void put_element_to_clean_row_with_params(unsigned int nob, unsigned int index,
                                          unsigned int elts_per_word, unsigned int *row, unsigned int elt)
{
  unsigned int word_offset = index / elts_per_word;
  unsigned int bit_offset = (index % elts_per_word) * nob;
  unsigned int word;
  row += word_offset;
  word = *row;
  assert(0 != nob);
  assert(NULL != row);
  assert(bit_offset + nob <= bits_in_unsigned_int);
  elt = elt << bit_offset;
  word = word | elt;
  *row = word;
}

void put_element_to_char_row(unsigned int eperb, unsigned int prime,
                             unsigned int index, unsigned char *row, unsigned int elt)
{
  unsigned int char_offset;
  unsigned int field;
  unsigned int word;
  unsigned int i = 0, q = 1, n, m;
  assert(0 != eperb);
  assert(0 != prime);
  assert(NULL != row);
  assert(elt < prime);
  char_offset = index / eperb;
  field = (eperb - 1 - (index % eperb));
  word = row[char_offset];
  while (i < field) {
    q *= prime;
    i++;
  }
  m = (1 == q) ? 0 : word % q;
  n = word / (q * prime);
  word = n * (q * prime) + elt * q + m;
  row[char_offset] = word;
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

/* Count the non-zero elements in a word */
unsigned int count_word(unsigned int word, unsigned int nob)
{
  unsigned int mask = (1 << nob) - 1;
  unsigned int res = 0;
  while (0 != word) {
    if (0 != (word & mask)) {
      res++;
    }
    word >>= nob;
  }
  return res;
}

unsigned int negate_elements(unsigned int elts, unsigned int nob, unsigned int prime)
{
  unsigned int new = 0, i = 0, mask = (1 << nob) - 1;
  if (0 == inited && 0 == primes_init(prime, &prime_operations)) {
    return 0;
  }
  inited = 1;
  while (0 != elts) {
    unsigned int elt = elts & mask;
    new |= ((*prime_operations.negate)(elt)) << (i * nob);
    i++;
    elts >>= nob;
  }
  return new;
}

unsigned int invert_elements(unsigned int elts, unsigned int nob, unsigned int prime)
{
  unsigned int new = 0, i = 0, mask = (1 << nob) - 1;
  if (0 == inited && 0 == primes_init(prime, &prime_operations)) {
    return 0;
  }
  inited = 1;
  while (0 != elts) {
    unsigned int elt = elts & mask;
    new |= ((*prime_operations.invert)(elt)) << (i * nob);
    i++;
    elts >>= nob;
  }
  return new;
}

unsigned int first_non_zero(unsigned int *row, unsigned int nob,
                            unsigned int len, unsigned int *pos)
{
  unsigned int i = 0;
  unsigned int elts_per_word = bits_in_unsigned_int / nob;
  assert(NULL != row);
  assert(NULL != pos);
  while (0 != len) {
    unsigned int elts = *row;
    if (0 != elts) {
      unsigned int mask = (1 << nob) - 1;
      while (0 != elts) {
        unsigned int elt = elts & mask;
        if (0 != elt) {
          *pos = i;
          return elt;
        } else {
          elts >>= nob;
          i++;
        }
      }
      assert(0);
    } else {
      i += elts_per_word;
      row++;
      len--;
    }
  }
  return 0; /* No first non zero found */
}
