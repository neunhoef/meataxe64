/*
 * $Id: count.c,v 1.1 2001/10/06 23:33:12 jon Exp $
 *
 * Function to count the non-zero elements in a matrix
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include "elements.h"
#include "endian.h"
#include "header.h"
#include "memory.h"
#include "read.h"
#include "count.h"

unsigned int count(const char *matrix, const char *name)
{
  FILE *input;
  unsigned int prime, noc, nor, nob, len;
  unsigned int i, j;
  const header *h;
  unsigned total = 0;
  unsigned int *row;

  endian_init();
  memory_init(name, 0);
  input = fopen(matrix, "rb");
  if (NULL == input) {
    fprintf(stderr, "%s: cannot open %s, terminating\n", name, matrix);
    exit(1);
  }
  if (0 == read_binary_header(input, &h, matrix)) {
    fclose(input);
    exit(1);
  }
  prime = header_get_prime(h);
  nob = header_get_nob(h);
  nor = header_get_nor(h);
  noc = header_get_noc(h);
  len = header_get_len(h);
  if (memory_rows(len, 1000) < 1) {
    fprintf(stderr, "%s: cannot allocate row for %s, terminating\n", name, matrix);
    fclose(input);
    exit(1);
  }
  row = memory_pointer_offset(0, 0, len);
  for (i = 0; i < nor; i++) {
    if (0 == endian_read_row(input, row, len)) {
      fprintf(stderr, "%s: cannot read row %d from %s, terminating\n", name, i, matrix);
      fclose(input);
      exit(1);
    }
    for (j = 0; j < len; j++) {
      total += count_word(row[j], nob);
    }
  }
  fclose(input);
  memory_dispose();
  return total;
}
