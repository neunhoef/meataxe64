/*
 * $Id: add.c,v 1.11 2001/11/25 12:44:32 jon Exp $
 *
 * Function to add two matrices to give a third
 *
 */

#include "add.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <limits.h>
#include "elements.h"
#include "endian.h"
#include "header.h"
#include "memory.h"
#include "read.h"
#include "rows.h"
#include "utils.h"
#include "write.h"

int add(const char *m1, const char *m2, const char *m3, const char *name)
{
  FILE *inp1 = NULL;
  FILE *inp2 = NULL;
  FILE *outp = NULL;
  unsigned int prime, nob, noc, nor, len;
  unsigned int i;
  const header *h1, *h2;
  unsigned int *row1, *row2;
  row_ops row_operations;
  row_incer incer;

  endian_init();
  if (0 == open_and_read_binary_header(&inp1, &h1, m1, name) ||
      0 == open_and_read_binary_header(&inp2, &h2, m2, name)) {
    if (NULL != inp1) {
      fclose(inp1);
    }
    return 0;
  }
  prime = header_get_prime(h1);
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
    return 0;
  }
  if (0 == rows_init(prime, &row_operations)) {
    fprintf(stderr, "%s: cannot initialise row operations for %s, %s, terminating\n", name, m1, m2);
    fclose(inp1);
    fclose(inp2);
    return 0;
  }
  incer = row_operations.incer;
  if (0 == open_and_write_binary_header(&outp, h1, m3, name)) {
    fclose(inp1);
    fclose(inp2);
    return 0;
  }
  if (memory_rows(len, 1000) < 2) {
    fprintf(stderr, "%s cannot allocate rows for %s, %s, %s, terminating\n", name, m1, m2, m3);
    fclose(inp1);
    fclose(inp2);
    fclose(outp);
    return 0;
  }
  row1 = memory_pointer_offset(0, 0, len);
  row2 = memory_pointer_offset(0, 1, len);
  for (i = 0; i < nor; i++) {
    if (0 == endian_read_row(inp1, row1, len)) {
      fprintf(stderr, "%s cannot read row %d from %s, terminating\n", name, i, m1);
      fclose(inp1);
      fclose(inp2);
      fclose(outp);
      return 0;
    }
    if (0 == endian_read_row(inp2, row2, len)) {
      fprintf(stderr, "%s cannot read row %d from %s, terminating\n", name, i, m2);
      fclose(inp1);
      fclose(inp2);
      fclose(outp);
      return 0;
    }
    (*incer)(row1, row2, len);
    if (0 == endian_write_row(outp, row2, len)) {
      fprintf(stderr, "%s cannot write row %d to %s, terminating\n", name, i, m3);
      fclose(inp1);
      fclose(inp2);
      fclose(outp);
      return 0;
    }
  }
  fclose(inp1);
  fclose(inp2);
  fclose(outp);
  return 1;
}
