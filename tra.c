/*
 * $Id: tra.c,v 1.5 2001/11/25 12:44:33 jon Exp $
 *
 * Function to transpose a matrix
 *
 */

#include "tra.h"
#include <stdio.h>
#include <assert.h>
#include "endian.h"
#include "elements.h"
#include "header.h"
#include "matrix.h"
#include "memory.h"
#include "read.h"
#include "write.h"

int tra(const char *m1, const char *m2, const char *name)
{
  FILE *input;
  FILE *output;
  unsigned int nob, noc, nor, len1, len2, max, total, t1;
  unsigned int i, j, k, l;
  const header *h1, *h2;
  unsigned int *row1;
  unsigned int **rows;
  long pos;

  assert(NULL != m1);
  assert(NULL != m2);
  assert(NULL != name);
  endian_init();
  if (0 == open_and_read_binary_header(&input, &h1, m1, name)) {
    return 0;
  }
  nob = header_get_nob(h1);
  nor = header_get_nor(h1);
  noc = header_get_noc(h1);
  len1 = header_get_len(h1);
  /* Create header for transpose */
  h2 = header_create(header_get_prime(h1), nob, header_get_nod(h1), nor, noc);
  len2 = header_get_len(h2);
  /* Maximum row size */
  max = (len1 > len2) ? len1 : len2;
  if (0 == open_and_write_binary_header(&output, h2, m2, name)) {
    fprintf(stderr, "%s cannot open or write header to %s, terminating\n", name, m2);
    fclose(input);
    return 0;
  }
  total = memory_rows(max, 1000);
  if (total < 2) {
    fprintf(stderr, "%s cannot allocate rows for %s, %s, terminating\n", name, m1, m2);
    fclose(input);
    fclose(output);
    return 0;
  }
  t1 = total - 1;
  rows = matrix_malloc(t1);
  row1 = memory_pointer_offset(0, 0, max);
  for (i = 0; i < t1; i++) {
      rows[i] = memory_pointer_offset(0, i + 1, max);
  }
  for (i = 0; i < noc; i += t1) {
    /* Number of output rows at once */
    k = (i + t1 > noc) ? noc - i : t1;
    pos = ftell(input);
    for (j = 0; j < nor; j++) {
      /* Read one row */
      if (0 == endian_read_row(input, row1, len1)) {
          fprintf(stderr, "%s cannot read row %d from %s, terminating\n", name, i, m1);
          fclose(input);
          fclose(output);
          return 0;
      }
      /* Now write into k output rows starting at column j */
      for (l = i; l < i + k; l++) {
        /* Write into row l of output at column j */
        put_element_to_row(nob, j, rows[l - i], get_element_from_row(nob, l, row1));
      }
    }
    if (0 != fseek(input, pos, SEEK_SET)) {
      fprintf(stderr, "%s: unable to rewind %s, terminating\n", name, m1);
      fclose(input);
      fclose(output);
      return 0;
    }
    for (j = 0; j < k; j++) {
      if (0 == endian_write_row(output, rows[j], len2)) {
        fprintf(stderr, "%s cannot write row %d to %s, terminating\n", name, j, m2);
        fclose(input);
        fclose(output);
        return 0;
      }
    }
  }
  fclose(input);
  fclose(output);
  return 1;
}
