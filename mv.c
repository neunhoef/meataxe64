/*
 * $Id: mv.c,v 1.3 2002/10/13 16:38:07 jon Exp $
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

void v_to_m(unsigned int *row_in, unsigned int **rows_out,
            unsigned int nor1, unsigned int nor2,
            unsigned int prime)
{
  const header *h;
  unsigned int i, j, len, nob, mask, elts_per_word;
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
      unsigned int k = i * nor2;
      unsigned int elt = get_element_from_row_with_params(nob, k + j, mask, elts_per_word, row_in);
      if (elt) {
        put_element_to_clean_row_with_params(nob, j, elts_per_word, rows_out[i], elt);
      }
    }
  }
  header_free(h);
}

extern void m_to_v(unsigned int **rows_in, unsigned int *row_out,
                   unsigned int nor, unsigned int noc,
                   unsigned int prime)
{
  const header *h;
  unsigned int i, j, len, nob, mask, elts_per_word;
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
      unsigned int k = i * noc;
      unsigned int elt = get_element_from_row_with_params(nob, j, mask, elts_per_word, rows_in[i]);
      if (elt) {
        put_element_to_clean_row_with_params(nob, k + j, elts_per_word, row_out, elt);
      }
    }
  }
  header_free(h);
}

void create_pointers(unsigned int *row_in, unsigned int **rows_out,
                     unsigned int nor, unsigned int len,
                     unsigned int prime)
{
  unsigned int i, nob;
  assert(NULL != row_in);
  assert(NULL != rows_out);
  assert(is_a_prime_power(prime));
  nob = bits_of(prime);
  for (i = 0; i < nor; i++) {
    rows_out[i] = row_in + i * len;
  }
}
