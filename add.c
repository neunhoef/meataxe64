/*
 * $Id: add.c,v 1.1 2001/08/30 18:31:45 jon Exp $
 *
 * Function to add two matrices to give a third
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <limits.h>
#include "header.h"
#include "utils.h"
#include "read.h"
#include "write.h"
#include "elements.h"
#include "endian.h"
#include "rows.h"
#include "add.h"

int add(const char *m1, const char *m2, const char *m3)
{
  FILE *inp1;
  FILE *inp2;
  FILE *outp;
  unsigned int prime, nob, noc, nor;
  unsigned int i;
  header h1, h2;
  unsigned int *row1, *row2, *row3;
  unsigned int row_words;
  unsigned int row_chars;
  unsigned int elts_per_word;

  endian_init();
  inp1 = fopen(m1, "rb");
  if (NULL == inp1) {
    fprintf(stderr, "ad: cannot open %s, terminating\n", m1);
    return 0;
  }
  inp2 = fopen(m2, "rb");
  if (NULL == inp2) {
    fprintf(stderr, "ad: cannot open %s, terminating\n", m2);
    fclose(inp1);
    return 0;
  }
  outp = fopen(m3, "wb");
  if (NULL == outp) {
    fprintf(stderr, "ad: cannot open %s, terminating\n", m3);
    fclose(inp1);
    fclose(inp2);
    return 0;
  }
  if (0 == read_binary_header(inp1, &h1, m1) ||
      0 == read_binary_header(inp2, &h2, m2)) {
    fclose(inp1);
    fclose(inp2);
    fclose(outp);
    return 0;
  }
  prime = header_get_prime(h1);
  nob = header_get_nob(h1);
  nor = header_get_nor(h1);
  noc = header_get_noc(h1);
  if (header_get_prime(h2) != prime ||
      header_get_nob(h2) != nob ||
      header_get_noc(h2) != noc ||
      header_get_nor(h2) != nor) {
    fprintf(stderr, "ad: header mismatch between %s and %s, terminating\n", m1, m2);
    fclose(inp1);
    fclose(inp2);
    fclose(outp);
    return 0;
  }
  write_binary_header(outp, h1, m3);
  elts_per_word = bits_in_unsigned_int / nob;
  row_words = (noc + elts_per_word - 1) / elts_per_word;
  row_chars = row_words * sizeof(unsigned int);
  row1 = malloc(row_chars);
  row2 = malloc(row_chars);
  row3 = malloc(row_chars);
  if (NULL == row1 || NULL == row2 || NULL == row3) {
    fprintf(stderr, "ad: cannot allocate rows for %s, %s, %s, terminating\n", m1, m2, m3);
    fclose(inp1);
    fclose(inp2);
    fclose(outp);
    return 0;
  }
  for (i = 0; i < nor; i++) {
    if (0 == endian_read_row(inp1, row1, nob, noc)) {
      fprintf(stderr, "ad: cannot read row %d from %s, terminating\n", i, m1);
      fclose(inp1);
      fclose(inp2);
      fclose(outp);
      return 0;
    }
    if (0 == endian_read_row(inp2, row2, nob, noc)) {
      fprintf(stderr, "ad: cannot read row %d from %s, terminating\n", i, m2);
      fclose(inp1);
      fclose(inp2);
      fclose(outp);
      return 0;
    }
    if (0 == row_add(row1, row2, row3, prime, nob, noc)) {
      fprintf(stderr, "ad: addition not supported for %d, terminating\n", prime);
      fclose(inp1);
      fclose(inp2);
      fclose(outp);
      return 0;
    }
    if (0 == endian_write_row(outp, row3, nob, noc)) {
      fprintf(stderr, "ad: cannot write row %d to %s, terminating\n", i, m3);
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
