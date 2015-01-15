/*
 * $Id: mv.c,v 1.6 2015/01/15 09:06:45 jon Exp $
 *
 * Function to convert rows to matrices and vv
 * Used for multiplication in tensor space
 *
 */

#include "mv.h"
#include <stdio.h>
#include <assert.h>
#include "elements.h"
#include "header.h"
#include "primes.h"
#include "rows.h"
#include "utils.h"

void v_to_m(word *row_in, word **rows_out,
            u32 nor1, u32 nor2,
            u32 prime)
{
  const header *h;
  u32 i, j, len, nob, elts_per_word;
  word mask;
  assert(NULL != row_in);
  assert(NULL != rows_out);
  assert(is_a_prime_power(prime));
  /* Form a header describing the output matrix */
  nob = bits_of(prime);
  mask = get_mask_and_elts(nob, &elts_per_word);
  /* Create a header for rows with nor2 columns, and nor1 rows */
  h = header_create(prime, nob, digits_of(prime), nor2, nor1);
  len = header_get_len(h);
  for (i = 0; i < nor1; i++) {
    row_init(rows_out[i], len);
    for (j = 0; j < nor2; j++) {
      u32 k = i * nor2;
      word elt = get_element_from_row_with_params(nob, k + j, mask, elts_per_word, row_in);
      if (elt) {
        put_element_to_clean_row_with_params(nob, j, elts_per_word, rows_out[i], elt);
      }
    }
  }
  header_free(h);
}

void m_to_v(word **rows_in, word *row_out,
                   u32 nor, u32 noc,
                   u32 prime)
{
  const header *h;
  u32 i, j, len, nob, elts_per_word;
  word mask;
  assert(NULL != rows_in);
  assert(NULL != row_out);
  assert(is_a_prime_power(prime));
  /* Form a header describing the output matrix */
  nob = bits_of(prime);
  mask = get_mask_and_elts(nob, &elts_per_word);
  h = header_create(prime, nob, digits_of(prime), nor * noc, 1);
  len = header_get_len(h);
  row_init(row_out, len);
  for (i = 0; i < nor; i++) {
    for (j = 0; j < noc; j++) {
      u32 k = i * noc;
      word elt = get_element_from_row_with_params(nob, j, mask, elts_per_word, rows_in[i]);
      if (elt) {
        put_element_to_clean_row_with_params(nob, k + j, elts_per_word, row_out, elt);
      }
    }
  }
  header_free(h);
}

void create_pointers(word *row_in, word **rows_out,
                     u32 nor, u32 len)
{
  u32 i;
  assert(NULL != row_in);
  assert(NULL != rows_out);
  for (i = 0; i < nor; i++) {
    rows_out[i] = row_in + i * len;
  }
}
