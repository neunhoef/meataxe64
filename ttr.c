/*
 * $Id: ttr.c,v 1.3 2005/07/24 09:32:45 jon Exp $
 *
 * Function to transpose some tensor product vectors
 *
 */

#include "ttr.h"
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

int ttr(u32 input_noc, const char *m1, const char *m2, const char *name)
{
  FILE *input;
  FILE *output;
  u32 nob, noc, nor, prime, len, input_nor;
  u32 i, elts_per_word;
  const header *h1;
  word mask, *row1, *row2;

  assert(NULL != m1);
  assert(NULL != m2);
  assert(NULL != name);
  assert(0 != input_noc);
  if (0 == open_and_read_binary_header(&input, &h1, m1, name)) {
    return 0;
  }
  prime = header_get_prime(h1);
  nor = header_get_nor(h1);
  noc = header_get_noc(h1);
  nob = header_get_nob(h1);
  len = header_get_len(h1);
  if (1 == prime) {
    /* Handle is a map */
    fprintf(stderr, "%s: cannot tensor transpose map %s\n", name, m1);
    fclose(input);
    header_free(h1);
    return 0;
  }
  if (0 != noc % input_noc) {
    fprintf(stderr, "%s: %s has %u columns which is not divisible by %u\n", name, m1, noc, input_noc);
    fclose(input);
    header_free(h1);
    return 0;
  }
  input_nor = noc / input_noc;
  /* Create header for tensor transpose (same parameters) */
  if (0 == open_and_write_binary_header(&output, h1, m2, name)) {
    fclose(input);
    header_free(h1);
    return 0;
  }
  header_free(h1);
  if (memory_rows(len, 1000) < 2) {
    fprintf(stderr, "%s cannot allocate rows for %s, %s, terminating\n", name, m1, m2);
    fclose(input);
    fclose(output);
    return 0;
  }
  row1 = memory_pointer_offset(0, 0, len);
  row2 = memory_pointer_offset(0, 1, len);
  mask = get_mask_and_elts(nob, &elts_per_word);
  for (i = 0; i < nor; i++) {
    u32 j, k, l;
    /* Read one row */
    errno = 0;
    if (0 == endian_read_row(input, row1, len)) {
      if ( 0 != errno) {
        perror(name);
      }
      fprintf(stderr, "%s cannot read row %u from %s, terminating\n", name, i, m1);
      fclose(input);
      fclose(output);
      return 0;
    }
    /* Init output row */
    row_init(row2, len);
    /* Create output row */
    for (j = 0; j < input_nor; j++) {
      for (k = 0; k < input_noc; k++) {
        word elt;
        l = j * input_noc + k;
        elt = get_element_from_row_with_params(nob, l, mask, elts_per_word, row1);
        if (0 != elt) {
          l = k * input_nor + j;
          put_element_to_clean_row_with_params(nob, l, elts_per_word, row2, elt);
        }
      }
    }
    /* Write one row */
    errno = 0;
    if (0 == endian_write_row(output, row2, len)) {
      if ( 0 != errno) {
        perror(name);
      }
      fprintf(stderr, "%s cannot write row %u to %s, terminating\n", name, i, m2);
      fclose(input);
      fclose(output);
      return 0;
    }
  }
  fclose(input);
  fclose(output);
  return 1;
}
