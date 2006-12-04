/*
 * $Id: zspan.c,v 1.17 2006/12/04 22:33:25 jon Exp $
 *
 * Compute the span of a matrix
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "endian.h"
#include "header.h"
#include "matrix.h"
#include "memory.h"
#include "parse.h"
#include "read.h"
#include "rows.h"
#include "span.h"
#include "utils.h"
#include "write.h"

static const char *name = "zspan";

static void span_usage(void)
{
  fprintf(stderr, "%s: usage: %s %s <in_file> <out_file> <n>\n", name, name, parse_usage());
}

int main(int argc, const char * const argv[])
{
  const char *in;
  const char *out;
  FILE *inp;
  FILE *outp;
  u32 prime, nob, noc, rows, len;
  u32 i, vectors;
  const header *h_in, *h_out;
  word **mat, *row, *scalars;
  row_ops row_operations;

  argv = parse_line(argc, argv, &argc);
  if (4 != argc) {
    span_usage();
    exit(1);
  }
  in = argv[1];
  out = argv[2];
  endian_init();
  vectors = strtoul(argv[3], NULL, 0);
  if (0 == vectors) {
    vectors = 1;
  }
  if (0 == open_and_read_binary_header(&inp, &h_in, in, name)) {
    exit(1);
  }
  prime = header_get_prime(h_in);
  if (1 == prime) {
    fprintf(stderr, "%s: cannot handle maps, terminating\n", name);
    fclose(inp);
    header_free(h_in);
    exit(1);
  }
  noc = header_get_noc(h_in);
  nob = header_get_nob(h_in);
  len = header_get_len(h_in);
  if (0 == int_pow(prime, vectors, &rows)) {
    fprintf(stderr, "%s: cannot compute %u ** %u, terminating\n", name, prime, vectors);
    exit(1);
  }
  rows--;
  rows /= (prime - 1); /* Only want projective representatives */
  h_out = header_create(prime, nob, header_get_nod(h_in), noc, rows);
  if (0 == open_and_write_binary_header(&outp, h_out, out, name)) {
    exit(1);
  }
  memory_init(name, memory);
  header_free(h_in);
  header_free(h_out);
  if (0 == rows_init(prime, &row_operations)) {
    fprintf(stderr, "%s: cannot initialise row operations for %s, %s, terminating\n", name, in, out);
    fclose(inp);
    fclose(outp);
  }
  mat = matrix_malloc(vectors);
  if (memory_rows(len, 1000) < vectors + 1) {
    fprintf(stderr, "%s: cannot allocate %u rows for %s, terminating\n", name, vectors + 1, in);
    exit(1);
  }
  for (i = 0; i < vectors; i++) {
    mat[i] = memory_pointer_offset(0, i, len);
  }
  errno = 0;
  if (0 == endian_read_matrix(inp, mat, len, vectors)) {
    if ( 0 != errno) {
      perror(name);
    }
    fprintf(stderr, "%s: cannot read %s, terminating\n", name, in);
    exit(1);
  }
  row = memory_pointer_offset(0, vectors, len);
  scalars = my_malloc(vectors * sizeof(word));
  memset(scalars, 0, vectors * sizeof(word));
  for (i = 0; i < rows; i++) {
    u32 j;
    span(vectors, scalars, prime, &j);
    row_init(row, len);
    for (j = 0; j < vectors; j++) {
      if (0 != scalars[j]) {
        if (1 != scalars[j]) {
          (*row_operations.scaled_incer)(mat[j], row, len, scalars[j]);
        } else {
          (*row_operations.incer)(mat[j], row, len);
        }
      }
    }
    errno = 0;
    if (0 == endian_write_row(outp, row, len)) {
      if ( 0 != errno) {
        perror(name);
      }
      fprintf(stderr, "%s: failed to write row %u to %s, terminating\n", name, i, out);
      exit(1);
    }
  }
  fclose(inp);
  fclose(outp);
  matrix_free(mat);
  return 0;
}
