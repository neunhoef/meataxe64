/*
         zex.c   -   explode a matrix (ie chop it up, converse implode)
	 =====       version 6.0.1 20/10/98
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "map.h"
#include "endian.h"
#include "exrows.h"
#include "files.h"
#include "header.h"
#include "memory.h"
#include "read.h"
#include "utils.h"

static const char *name = "zex";

static void zex_usage(void)
{
  fprintf(stderr, "%s: usage: %s <mat> <dir> <int>\n", name, name);
}

int main(int argc,  char **argv)
{
  FILE *input;
  unsigned int split;
  unsigned int nob, nor, noc, len, prime;
  unsigned int col_pieces, row_pieces;
  unsigned int elts_per_word;
  const char **names;
  const header *h;
  unsigned int i;
  FILE **outputs;
  unsigned int *row;
  if (4 != argc) {
    zex_usage();
    exit(1);
  }
  memory_init(name, 0);
  split = strtoul(argv[3], NULL, 0);
  if (0 == split) {
    fprintf(stderr, "%s: required submatrix size must be non-zero\n", name);
    exit(1);
  }
  input = fopen(argv[1], "rb");
  if (NULL == input) {
    fprintf(stderr, "%s cannot open %s, terminating\n", name, argv[1]);
    exit(1);
  }
  if (0 == read_binary_header(input, &h, argv[1])) {
    fprintf(stderr, "%s cannot ead header from %s, terminating\n", name, argv[1]);
    exit(1);
  }
  prime = header_get_prime(h);
  nob = header_get_nob(h);
  nor = header_get_nor(h);
  noc = header_get_noc(h);
  len = header_get_len(h);
  elts_per_word = bits_in_unsigned_int / nob;
  /* Now compute the number of pieces for both rows and columns */
  /* And produce the description file */
  /* Align split to word boundary */
  split = ((split + elts_per_word - 1) / elts_per_word) * elts_per_word;
  row_pieces = (nor + split - 1) / split;
  col_pieces = (noc + split - 1) / split;
  outputs = my_malloc(col_pieces * sizeof(FILE *));
  output_map(name, argv[2], col_pieces, row_pieces, &names);
  if (memory_rows(len, 1000) < 2) {
    fprintf(stderr, "%s cannot allocate row for %s, terminating\n", name, argv[1]);
    fclose(input);
    exit(1);
  }
  row = memory_pointer_offset(0, 0, len);
  for (i = 0; i < nor; i++) {
    if (0 == endian_read_row(input, row, len)) {
      fprintf(stderr, "%s cannot read row %d from %s, terminating\n", name, i, argv[1]);
      fclose(input);
    }
    /* Write the row */
    ex_row_put(i, noc, argv[2], names, split, row, outputs);
  }
  fclose(input);
  memory_dispose();
  return 0;
}
