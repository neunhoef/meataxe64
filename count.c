/*
 * $Id: count.c,v 1.9 2002/10/14 19:11:51 jon Exp $
 *
 * Function to count the non-zero elements in a matrix
 *
 */

#include "count.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "elements.h"
#include "endian.h"
#include "header.h"
#include "memory.h"
#include "read.h"

unsigned int count(const char *matrix, const char *name)
{
  FILE *input;
  unsigned int prime, noc, nor, nob, len;
  unsigned int i, j;
  const header *h;
  unsigned total = 0;
  unsigned int *row;

  assert(NULL != matrix);
  if (0 == open_and_read_binary_header(&input, &h, matrix, name)) {
    exit(1);
  }
  prime = header_get_prime(h);
  nor = header_get_nor(h);
  noc = header_get_noc(h);
  if (1 == prime) {
    /* This is easy, it's just the minimum of nor, noc */
    fclose(input);
    header_free(h);
    return (nor > noc) ? noc : nor;
  }
  nob = header_get_nob(h);
  len = header_get_len(h);
  if (memory_rows(len, 1000) < 1) {
    fprintf(stderr, "%s: cannot allocate row for %s, terminating\n", name, matrix);
    fclose(input);
    header_free(h);
    exit(1);
  }
  row = memory_pointer_offset(0, 0, len);
  for (i = 0; i < nor; i++) {
    if (0 == endian_read_row(input, row, len)) {
      fprintf(stderr, "%s: cannot read row %d from %s, terminating\n", name, i, matrix);
      fclose(input);
      header_free(h);
      exit(1);
    }
    for (j = 0; j < len; j++) {
      total += count_word(row[j], nob);
    }
  }
  fclose(input);
  header_free(h);
  memory_dispose();
  return total;
}
