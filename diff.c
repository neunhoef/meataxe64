/*
 * $Id: diff.c,v 1.4 2002/06/25 10:30:12 jon Exp $
 *
 * Function to find the differences between two matrices
 *
 */

#include "diff.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "endian.h"
#include "header.h"
#include "memory.h"
#include "read.h"
#include "utils.h"

int diff(const char *m1, const char *m2, const char *name)
{
  FILE *inp1 = NULL;
  FILE *inp2 = NULL;
  unsigned int prime, nob, noc, nor, len;
  unsigned int i;
  int res = 1;
  const header *h1, *h2;
  unsigned int *row1, *row2;

  if (0 == open_and_read_binary_header(&inp1, &h1, m1, name) ||
      0 == open_and_read_binary_header(&inp2, &h2, m2, name)) {
    if (NULL != inp1) {
      fclose(inp1);
      header_free(h1);
    }
    return 0;
  }
  prime = header_get_prime(h1);
  if (1 == prime) {
    fprintf(stderr, "%s: cannot handle maps, terminating\n", name);
    fclose(inp1);
    fclose(inp2);
    header_free(h1);
    header_free(h2);
    return 0;
  }
  nob = header_get_nob(h1);
  nor = header_get_nor(h1);
  noc = header_get_noc(h1);
  len = header_get_len(h1);
  if (header_get_prime(h2) != prime ||
      header_get_nob(h2) != nob ||
      header_get_noc(h2) != noc ||
      header_get_nor(h2) != nor) {
    fprintf(stderr, "%s header mismatch between %s and %s, terminating\n", name, m1, m2);
    fclose(inp1);
    fclose(inp2);
    header_free(h1);
    header_free(h2);
    return 0;
  }
  header_free(h1);
  header_free(h2);
  if (memory_rows(len, 1000) < 2) {
    fprintf(stderr, "%s cannot allocate rows for %s, %s, terminating\n", name, m1, m2);
    fclose(inp1);
    fclose(inp2);
    return 0;
  }
  row1 = memory_pointer_offset(0, 0, len);
  row2 = memory_pointer_offset(0, 1, len);
  for (i = 0; i < nor; i++) {
    if (0 == endian_read_row(inp1, row1, len)) {
      fprintf(stderr, "%s cannot read row %d from %s, terminating\n", name, i, m1);
      fclose(inp1);
      fclose(inp2);
      return 0;
    }
    if (0 == endian_read_row(inp2, row2, len)) {
      fprintf(stderr, "%s cannot read row %d from %s, terminating\n", name, i, m2);
      fclose(inp1);
      fclose(inp2);
      return 0;
    }
    if (0 != memcmp(row1, row2, len * sizeof(unsigned int))) {
      unsigned int j, k = 0;
      for (j = 0; j < len; j++) {
        if (row1[j] != row2[j]) {
          printf("%s and %s differ in row %d near offset %d, with values 0x%x and 0x%x (diffs 0x%x)\n",
                 m1, m2, i, j * (bits_in_unsigned_int / nob), row1[j], row2[j], row1[j] ^ row2[j]);
          res = 0;
          k++;
          if (k > 10) {
            break;
          }
        }
      }
    }
  }
  fclose(inp1);
  fclose(inp2);
  return res;
}
