/*
 * $Id: tra.c,v 1.20 2019/01/21 08:32:35 jon Exp $
 *
 * Function to transpose a matrix
 *
 */

#include "tra.h"
#include <stdio.h>
#include <assert.h>
#include <errno.h>
#include "endian.h"
#include "elements.h"
#include "header.h"
#include "maps.h"
#include "matrix.h"
#include "memory.h"
#include "read.h"
#include "rows.h"
#include "utils.h"
#include "write.h"

void tra_in_situ(word **rows, u32 nor, u32 nob)
{
  u32 i, j, elts_per_word;
  word mask;
  assert(NULL != rows);
  mask = get_mask_and_elts(nob, &elts_per_word);
  for (i = 0; i + 1 < nor; i++) {
    for (j = i; j < nor; j++) {
      word elt1 = get_element_from_row_with_params(nob, j, mask, elts_per_word, rows[i]);
      word elt2 = get_element_from_row_with_params(nob, i, mask, elts_per_word, rows[j]);
      put_element_to_row_with_params(nob, i, mask, elts_per_word, rows[j], elt1);
      put_element_to_row_with_params(nob, j, mask, elts_per_word, rows[i], elt2);
    }
  }
}

void tra_in_store(word **rows1, word **rows2,
                  u32 nor, u32 noc,
                  u32 nob, u32 col_len)
{
  u32 i, j, elts_per_word;
  word mask;
  assert(NULL != rows1);
  assert(NULL != rows2);
  for (i = 0; i < noc; i++) {
    assert(NULL != rows2[i]);
    row_init(rows2[i], col_len);
  }
  mask = get_mask_and_elts(nob, &elts_per_word);
  for (i = 0; i < nor; i++) {
    for (j = 0; j < noc; j++) {
      word elt = get_element_from_row_with_params(nob, j, mask, elts_per_word, rows1[i]);
      if (elt) {
        put_element_to_clean_row_with_params(nob, i, elts_per_word, rows2[j], elt);
      }
    }
  }
}

int tra(const char *m1, const char *m2, const char *name)
{
  FILE *input;
  FILE *output;
  u32 nob, noc, nor, prime, len1, len2, max, total, t1;
  u32 i, j, k, l, elts_per_word;
  word mask;
  const header *h1, *h2;
  word *row1;
  word **rows;
  s64 pos;

  assert(NULL != m1);
  assert(NULL != m2);
  assert(NULL != name);
  if (0 == open_and_read_binary_header(&input, &h1, m1, name)) {
    return 0;
  }
  prime = header_get_prime(h1);
  nor = header_get_nor(h1);
  noc = header_get_noc(h1);
  if (1 == prime) {
    /* Handle is a map */
    int ret = map_iv(input, h1, m1, m2, name);
    header_free(h1);
    fclose(input);
    return ret;
  }
  nob = header_get_nob(h1);
  len1 = header_get_len(h1);
  /* Create header for transpose */
  h2 = header_create(prime, nob, header_get_nod(h1), nor, noc);
  header_free(h1);
  len2 = header_get_len(h2);
  /* Maximum row size */
  max = (len1 > len2) ? len1 : len2;
  if (0 == open_and_write_binary_header(&output, h2, m2, name)) {
    fclose(input);
    header_free(h2);
    return 0;
  }
  header_free(h2);
  total = memory_rows(max, 1000);
  if (total < 2) {
    fprintf(stderr, "%s cannot allocate rows for %s, %s, terminating\n", name, m1, m2);
    fclose(input);
    fclose(output);
    return 0;
  }
  t1 = total - 1;
  if (t1 > nor + noc) {
    t1 = nor + noc;
  }
  rows = matrix_malloc(t1);
  row1 = memory_pointer_offset(0, 0, max);
  for (i = 0; i < t1; i++) {
    rows[i] = memory_pointer_offset(0, i + 1, max);
  }
  mask = get_mask_and_elts(nob, &elts_per_word);
  for (i = 0; i < noc; i += t1) {
    /* Number of output rows at once */
    k = (i + t1 > noc) ? noc - i : t1;
    pos = ftello(input);
    /* Initialise all output rows */
    for (j = 0; j < k; j++) {
      row_init(rows[j], len2);
    }
    for (j = 0; j < nor; j++) {
      /* Read one row */
      errno = 0;
      if (0 == endian_read_row(input, row1, len1)) {
        if ( 0 != errno) {
          perror(name);
        }
        fprintf(stderr, "%s cannot read row %u from %s, terminating\n", name, i, m1);
        fclose(input);
        fclose(output);
        matrix_free(rows);
        return 0;
      }
      /* Now write into k output rows starting at column j */
      for (l = i; l < i + k; l++) {
        /* Write into row l of output at column j */
        word elt = get_element_from_row_with_params(nob, l, mask, elts_per_word, row1);
        if (0 != elt) {
          put_element_to_clean_row_with_params(nob, j, elts_per_word, rows[l - i], elt);
        }
      }
    }
    if (0 != fseeko(input, pos, SEEK_SET)) {
      fprintf(stderr, "%s: unable to rewind %s, terminating\n", name, m1);
      fclose(input);
      fclose(output);
      matrix_free(rows);
      return 0;
    }
    for (j = 0; j < k; j++) {
      errno = 0;
      if (0 == endian_write_row(output, rows[j], len2)) {
        if ( 0 != errno) {
          perror(name);
        }
        fprintf(stderr, "%s cannot write row %u to %s, terminating\n", name, j, m2);
        fclose(input);
        fclose(output);
        matrix_free(rows);
        return 0;
      }
    }
  }
  fclose(input);
  fclose(output);
  matrix_free(rows);
  return 1;
}
