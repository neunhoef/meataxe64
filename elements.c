/*
 * $Id: elements.c,v 1.31 2018/04/18 19:29:03 jon Exp $
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

static prime_ops prime_operations = {0, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
static int inited = 0;

int get_element_from_text(FILE *fp, u32 nod,
                          u32 prime, word *el)
{
  word e = 0;

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

word get_element_from_row(u32 nob, u32 index,
                          const word *row)
{
  u32 elts_per_word = bits_in_word / nob;
  u32 word_offset = index / elts_per_word;
  u32 bit_offset = (index % elts_per_word) * nob;
  word elt = row[word_offset];
  word bit = 1; /* Make sure C doesn't truncate the shift */
  word mask = (bit << nob) - 1;
  word res = (elt >> bit_offset) & mask;
  assert(0 != nob);
  assert(NULL != row);
  assert(bit_offset + nob <= bits_in_word);
  return res;
}

word get_element_from_char_row(u32 eperb, u32 prime,
                               u32 index, const unsigned char *row)
{
  u32 char_offset;
  u32 field;
  word word;
  u32 i = 0;
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

word get_mask_and_elts(u32 nob, u32 *elts_per_word)
{
  word bit = 1;
  assert(NULL != elts_per_word);
  *elts_per_word = bits_in_word / nob;
  return (bit << nob) - 1;
}

word get_element_from_row_with_params(u32 nob, u32 index, word mask,
                                      u32 elts_per_word, const word *row)
{
  u32 word_offset = index / elts_per_word;
  u32 bit_offset = (index % elts_per_word) * nob;
  word elt = row[word_offset];
  word res = (elt >> bit_offset) & mask;
  assert(0 != nob);
  assert(NULL != row);
  assert(bit_offset + nob <= bits_in_word);
  return res;
}

void get_elements_from_row_with_params_into_row(u32 nob, u32 w_o, u32 e_o, u32 b_o, word mask,
                                                u32 elts_per_word, const word *row,
                                                u32 count, word *out)
{
  u32 word_offset = w_o;
  u32 elt_offset = e_o;
  u32 bit_offset = b_o;
  word w;
  assert(0 != nob);
  assert(NULL != row);
  assert(NULL != out);
  row += word_offset;
  if (0 != elt_offset) {
    u32 sub_count = elts_per_word - elt_offset;
    word *end;
    if (sub_count > count) {
      sub_count = count;
    }
    end = out + sub_count;
    count -= sub_count;
    w = *row >> bit_offset;
    while (out < end) {
      *out = w & mask;
      w >>= nob;
      out++;
    }
    row++;
  }
  while (count >= elts_per_word) {
    word *end = out + elts_per_word;
    w = *row;
    while (out < end) {
      *out = w & mask;
      w >>= nob;
      out++;
    }
    row++;
    count -= elts_per_word;
  }
  if (count > 0) {
    word *end = out + count;
    w = *row;
    while (out < end) {
      *out = w & mask;
      w >>= nob;
      out++;
    }
  }
}

void element_access_init(u32 nob, u32 from, u32 size,
                         u32 *word_offset, u32 *bit_offset,
                         word *mask)
{
  u32 elts_per_word;
  u32 bits = nob * size;
  word bit = 1; /* We use this for the shift below */
  assert(0 != nob);
  assert(0 != size);
  assert(bits < bits_in_word);
  elts_per_word = bits_in_word / nob;
  *word_offset = from / elts_per_word;
  *bit_offset = (from % elts_per_word) * nob;
  /*
   * Avoid getting a shorter shift by using bit instead of 1
   * Eg 1 might be 32 bits, whereas word is 64.
   * Then 1 << 32 gives 0 rather than 0x100000000
   * Isn't C wonderful?
   */
  *mask = (bit << bits) - 1;
}

word get_elements_in_word_from_row(const word *row,
                                   u32 bit_offset, word mask)
{
  assert(NULL != row);
  return (*row >> bit_offset) & mask;
}

word get_elements_out_word_from_row(const word *row,
                                    u32 shift,
                                    u32 bit_offset, word mask)
{
  word word1, word2;
  assert(NULL != row);
  word1 = *row;
  word2 = (word1 >> bit_offset);
  word1 = (row[1] << shift) & mask;
  return word2 | word1;
}

/* This allows cross word access */
word get_elements_from_row(const word *row, u32 width,
                           u32 nob,
                           u32 bit_offset, word mask)
{
  word word1, word2;
  assert(NULL != row);
  assert(0 != width);
  word1 = *row;
  word2 = (word1 >> bit_offset) & mask;
  if (bit_offset + width <= bits_in_word) {
    return word2;
  } else {
    u32 elts_in_word1 = (bits_in_word - bit_offset) / nob;
    word1 = (row[1] << (elts_in_word1 * nob)) & mask;
    return word2 | word1;
  }
}

void put_element_to_row(u32 nob, u32 index,
                        word *row, word elt)
{
  u32 elts_per_word = bits_in_word / nob;
  u32 word_offset = index / elts_per_word;
  u32 bit_offset = (index % elts_per_word) * nob;
  word elt1 = row[word_offset];
  word bit = 1;
  word base_mask = (bit << nob) - 1;
  word mask = base_mask << bit_offset;
  assert(0 != nob);
  assert(NULL != row);
  assert(bit_offset + nob <= bits_in_word);
  elt = elt << bit_offset;
  elt1 = (elt1 & (mask ^ (~0))) | elt;
  row[word_offset] = elt1;
}

void put_element_to_row_with_params(u32 nob, u32 index, word mask,
                                    u32 elts_per_word, word *row, word elt)
{
  u32 word_offset = index / elts_per_word;
  u32 bit_offset = (index % elts_per_word) * nob;
  word word;
  row += word_offset;
  word = *row;
  mask = mask << bit_offset;
  assert(0 != nob);
  assert(NULL != row);
  assert(bit_offset + nob <= bits_in_word);
  elt = elt << bit_offset;
  word = (word & (~mask)) | elt;
  *row = word;
}

void put_element_to_clean_row_with_params(u32 nob, u32 index,
                                          u32 elts_per_word, word *row, word elt)
{
  u32 word_offset = index / elts_per_word;
  u32 bit_offset = (index % elts_per_word) * nob;
  word word;
  row += word_offset;
  word = *row;
  assert(0 != nob);
  assert(NULL != row);
  assert(bit_offset + nob <= bits_in_word);
  elt = elt << bit_offset;
  word = word | elt;
  *row = word;
}

void put_element_to_char_row(u32 eperb, u32 prime,
                             u32 index, unsigned char *row, word elt)
{
  u32 char_offset;
  u32 field;
  u32 word;
  u32 i = 0, q = 1, n, m;
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
word elements_contract(word elts, u32 prime, u32 nob)
{
  if (0 == prime % 2) {
    return elts; /* Exact representation for powers of 2 */
  } else {
    word bit = 1;
    word out = 0, power = 1, mask = (bit << nob) - 1;
    assert(0 != nob);
    assert(0 != prime);
    assert(prime <= mask);
    while (0 != elts) {
      word elt = elts & mask;
      out += power * elt;
      elts >>= nob;
      power *= prime;
    }
    return out;
  }
}

/* Count the non-zero elements in a word */
word count_word(word elt, u32 nob)
{
  word bit = 1;
  word mask = (bit << nob) - 1;
  word res = 0;
  while (0 != elt) {
    if (0 != (elt & mask)) {
      res++;
    }
    elt >>= nob;
  }
  return res;
}

word negate_elements(word elts, u32 nob, u32 prime)
{
  word bit = 1;
  word new = 0, i = 0, mask = (bit << nob) - 1;
  if (0 == inited && 0 == primes_init(prime, &prime_operations)) {
    return 0;
  }
  inited = 1;
  while (0 != elts) {
    word elt = elts & mask;
    new |= ((*prime_operations.negate)(elt)) << (i * nob);
    i++;
    elts >>= nob;
  }
  return new;
}

word invert_elements(word elts, u32 nob, u32 prime)
{
  word bit = 1;
  word new = 0, i = 0, mask = (bit << nob) - 1;
  if (0 == inited && 0 == primes_init(prime, &prime_operations)) {
    return 0;
  }
  inited = 1;
  while (0 != elts) {
    word elt = elts & mask;
    new |= ((*prime_operations.invert)(elt)) << (i * nob);
    i++;
    elts >>= nob;
  }
  return new;
}

u32 first_non_zero(word *row, u32 nob,
                   u32 len, u32 *pos)
{
  u32 i = 0;
  u32 elts_per_word = bits_in_word / nob;
  word bit = 1;
  word mask = (bit << nob) - 1;
  word *row_end = row + len;
  assert(NULL != row);
  assert(NULL != pos);
  while (row < row_end) {
    word elts = *row;
    if (0 != elts) {
      while (0 != elts) {
        word elt = elts & mask;
        if (0 != elt) {
          *pos = i;
          return elt;
        } else {
          elts >>= nob;
          i++;
        }
      }
      assert(assert_var_zero != 0);
    } else {
      i += elts_per_word;
      row++;
    }
  }
  return 0; /* No first non zero found */
}
u32 first_invertible(u32 prime, word *row, u32 nob,
                     u32 len, u32 *pos)
{
  u32 i = 0;
  u32 elts_per_word = bits_in_word / nob;
  word bit = 1;
  word mask = (bit << nob) - 1;
  word *row_end = row + len;
  assert(NULL != row);
  assert(NULL != pos);
  if (0 == inited && 0 == primes_init(prime, &prime_operations)) {
    return 0;
  }
  inited = 1;
  while (row < row_end) {
    word elts = *row;
    if (0 != elts) {
      u32 j = i;
      while (0 != elts) {
        word elt = elts & mask;
        if ((*prime_operations.is_invertible)(elt)) {
          *pos = j;
          return elt;
        } else {
          elts >>= nob;
          j++;
        }
      }
    }
    i += elts_per_word;
    row++;
  }
  return 0; /* No first non zero found */
}

u32 get_element_from_u32_row(u32 nob, u32 index,
                             const u32 *row)
{
  u32 elts_per_word = bits_in_u32 / nob;
  u32 word_offset = index / elts_per_word;
  u32 bit_offset = (index % elts_per_word) * nob;
  u32 elt = row[word_offset];
  u32 mask = (1 << nob) - 1;
  u32 res = (elt >> bit_offset) & mask;
  assert(0 != nob);
  assert(NULL != row);
  assert(bit_offset + nob <= bits_in_word);
  return res;
}

u64 get_element_from_u64_row(u32 nob, u32 index,
                             const u64 *row)
{
  u32 elts_per_word = bits_in_u64 / nob;
  u32 word_offset = index / elts_per_word;
  u32 bit_offset = (index % elts_per_word) * nob;
  u64 elt = row[word_offset];
  u64 bit = 1;
  u64 mask = (bit << nob) - 1;
  u64 res = (elt >> bit_offset) & mask;
  assert(0 != nob);
  assert(NULL != row);
  assert(bit_offset + nob <= bits_in_word);
  return res;
}

void put_element_to_u32_row(u32 nob, u32 index,
                            u32 *row, u32 elt)
{
  u32 elts_per_word = bits_in_u32 / nob;
  u32 word_offset = index / elts_per_word;
  u32 bit_offset = (index % elts_per_word) * nob;
  u32 elt1 = row[word_offset];
  u32 base_mask = (1 << nob) - 1;
  u32 mask = base_mask << bit_offset;
  assert(0 != nob);
  assert(NULL != row);
  assert(bit_offset + nob <= bits_in_word);
  elt = elt << bit_offset;
  elt1 = (elt1 & (mask ^ (~0))) | elt;
  row[word_offset] = elt1;
}

void put_element_to_u64_row(u32 nob, u32 index,
                            u64 *row, u64 elt)
{
  u32 elts_per_word = bits_in_u64 / nob;
  u32 word_offset = index / elts_per_word;
  u32 bit_offset = (index % elts_per_word) * nob;
  u64 elt1 = row[word_offset];
  u64 bit = 1;
  u64 base_mask = (bit << nob) - 1;
  u64 mask = base_mask << bit_offset;
  assert(0 != nob);
  assert(NULL != row);
  assert(bit_offset + nob <= bits_in_word);
  elt = elt << bit_offset;
  elt1 = (elt1 & (mask ^ (~0))) | elt;
  row[word_offset] = elt1;
}
