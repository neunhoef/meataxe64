/*
 * $Id: tra2.c,v 1.1 2012/02/07 22:00:45 jon Exp $
 *
 * Function to transpose a matrix
 *
 */

#include "tra2.h"
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

int tra2(const char *m1, const char *m2, const char *name)
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
  /* Try to read the input */
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
  /* Maximum row size of input and output */
  max = (len1 > len2) ? len1 : len2;
  /* Write the output header */
  if (0 == open_and_write_binary_header(&output, h2, m2, name)) {
    fclose(input);
    header_free(h2);
    return 0;
  }
  header_free(h2);
  /* How many of the biggest size can we fit? */
  total = memory_rows(max, 1000);
  if (total < 2) {
    fprintf(stderr, "%s cannot allocate rows for %s, %s, terminating\n", name, m1, m2);
    fclose(input);
    fclose(output);
    return 0;
  }
  t1 = total - 1;
  if (t1 > nor + noc) {
    /* Reduce t1 if we can get all of both in memory */
    t1 = nor + noc;
  }
  /* Allocate the rows we'll use */
  rows = matrix_malloc(t1);
  /* A spare row, what's this for? */
  row1 = memory_pointer_offset(0, 0, max);
  /* The rows for the input */
  for (i = 0; i < t1; i++) {
    rows[i] = memory_pointer_offset(0, i + 1, max);
  }
  mask = get_mask_and_elts(nob, &elts_per_word);
  /*
   * We want to do input rows in sets of 32
   * as we're going to transpose 32 rows by 32 bits
   * in 5 rounds of shifts, masks and ors
   */
#if 1
  NOT_USED(pos);
  NOT_USED(l);
  NOT_USED(k);
  NOT_USED(j);
  /* Algorithm */
  /*
    Compute n1 such that we can have n1 rows of length noc
    and n2 rows of length n1, 32 | (n1,n2)
    col_stride = n1
    cur_col = 0
    while (cur_col < noc) {
      cur_in_row = 0
      out_col = 0
      while (cur_in_row < nor) {
        Read n2 rows
        Transpose the n2 by n1 piece we've read (expand this)
        cur_in_row += n2
        out_col += n2
      }
      write out n1 rows
      cur_col += col_stride
    }
  */
#else
  for (i = 0; i < noc; i += t1) {
    /* Number of output rows at once */
    k = (i + t1 > noc) ? noc - i : t1;
    pos = ftello64(input);
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
    if (0 != fseeko64(input, pos, SEEK_SET)) {
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
#endif
  fclose(input);
  fclose(output);
  matrix_free(rows);
  return 1;
}
