/*
 * $Id: eim.c,v 1.1 2001/10/06 23:33:12 jon Exp $
 *
 * implode a matrix (ie glue exploded matrices together)
 *
 * Based on version 6.0.1 20/10/98
 */

#include <stdio.h>
#include <stdlib.h>
#include "elements.h"
#include "endian.h"
#include "files.h"
#include "header.h"
#include "memory.h"
#include "map.h"
#include "read.h"
#include "utils.h"
#include "write.h"

static const char *name = "eim";

static void eim_usage(void)
{
  fprintf(stderr, "%s: usage: %s <mat> <dir>\n", name, name);
}

static void fail(FILE **inputs, FILE *output, unsigned int cols)
{
  unsigned int i;
  for (i = 0; i < cols; i++) {
    fclose(inputs[i]);
  }
  fclose(output);
  exit(1);
}

int main(int argc,  char **argv)
{
  FILE *input, *output;
  FILE **inputs;
  const char *matrix;
  const header *h, *outh;
  const header **headers;
  unsigned int *row1, *row2;
  unsigned int nob, nod, nor, len, prime;
  unsigned int col_pieces, row_pieces;
  unsigned int rows, cols;
  unsigned int i, j;
  const char **names;
  if (3 != argc) {
    eim_usage();
    exit(1);
  }
  memory_init(name, 0);
  /* Now get a look at the map file */
  input_map(name, argv[1], &col_pieces, &row_pieces, &names);
  /* Now determine full size */
  rows = 0;
  cols = 0;
  for (i = 0; i < col_pieces; i++) {
    matrix = pathname(argv[2], names[i]);
    input = fopen(matrix, "rb");
    if (NULL == input) {
      fprintf(stderr, "%s: cannot open %s, terminating\n", name, matrix);
      exit(1);
    }
    if (0 == read_binary_header(input, &h, matrix)) {
      fprintf(stderr, "%s cannot read header from %s, terminating\n", name, matrix);
      fclose(input);
      exit(1);
    }
    fclose(input);
    cols += header_get_noc(h);
  }
  for (i = 0; i < row_pieces; i++) {
    matrix = pathname(argv[2], names[i * col_pieces]);
    input = fopen(matrix, "rb");
    if (NULL == input) {
      fprintf(stderr, "%s: cannot open %s, terminating\n", name, matrix);
      exit(1);
    }
    if (0 == read_binary_header(input, &h, matrix)) {
      fprintf(stderr, "%s cannot read header from %s, terminating\n", name, matrix);
      fclose(input);
      exit(1);
    }
    fclose(input);
    rows += header_get_nor(h);
  }
  prime = header_get_prime(h);
  nob = header_get_nob(h);
  nod = header_get_nod(h);
  outh = header_create(prime, nob, nod, cols, rows);
  len = header_get_len(outh);
  output = fopen(argv[1], "wb");
  if (NULL == output) {
    fprintf(stderr, "%s cannot open %s, terminating\n", name, argv[1]);
    exit(1);
  }
  if (0 == write_binary_header(output, outh, argv[1])) {
    fprintf(stderr, "%s cannot write header to %s, terminating\n", name, argv[1]);
    fclose(output);
    exit(1);
  }
  /* Now copy all the input matrices into the output one */
  if (memory_rows(len, 1000) < 2) {
    fprintf(stderr, "%s cannot allocate rows for %s, terminating\n", name, argv[1]);
    fclose(output);
    exit(1);
  }
  row1 = memory_pointer_offset(0, 0, len);
  row2 = memory_pointer_offset(0, 1, len);
  inputs = my_malloc(row_pieces * sizeof(*inputs));
  headers = my_malloc(row_pieces * sizeof(*headers));
  for (i = 0; i < row_pieces; i++) {
    unsigned int k;
    for (j = 0; j < col_pieces; j++) {
      const char *piece_name = names[i * row_pieces + j];
      inputs[j] = fopen(name, "rb");
      if (NULL == inputs[j]) {
        fprintf(stderr, "%s: cannot open %s, terminating\n", name, piece_name);
        exit(1);
      }
      if (0 == read_binary_header(inputs[j], headers + j, piece_name)) {
        fprintf(stderr, "%s cannot read header from %s, terminating\n", name, piece_name);
        fail(inputs, output, j + 1);
      }
    }
    /* Now check for consistent headers */
    nor = header_get_nor(headers[0]);
    for (j = 1; j < col_pieces; j++) {
      if (header_get_nor(headers[j]) != nor || header_get_prime(headers[j]) != prime) {
        /*** Check for consistency on prime */
        fprintf(stderr, "%s header mismatch %d != %d or %d != %d from %s and %s, terminating\n",
                name, nor, header_get_nor(headers[j]), prime, header_get_prime(headers[j]),
                names[i * col_pieces], names[i * col_pieces + j]);
        fail(inputs, output, col_pieces);
      }
    }
    for (k = 0; k < nor; k++) {
      unsigned int l = 0; /* Index into output row */
      for (j = 0; j < col_pieces; j++) {
        unsigned int m = 0, n;
        if (0 == endian_read_row(inputs[j], row2, header_get_len(headers[j]))) {
          fprintf(stderr, "%s cannot read row from %s, terminating\n",
                  name, names[i * col_pieces + j]);
          fail(inputs, output, col_pieces);
        }
        n = header_get_noc(headers[j]);
        while (m < n) {
          put_element_to_row(nob, l, row1, get_element_from_row(nob, m, row2));            
          m++;
          l++;
        }
      }
      if (0 == endian_write_row(output, row1, header_get_len(outh))) {
        fprintf(stderr, "%s cannot write row to %s, terminating\n",
                name, argv[1]);
        fail(inputs, output, col_pieces);
      }
    }
    /* Now close the current set and continue */
    for (j = 0; j < col_pieces; j++) {
      fclose(inputs[j]);
    }
  }
  fclose(output);
  memory_dispose();
  return 0;
}
