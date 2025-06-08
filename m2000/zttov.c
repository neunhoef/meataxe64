/*
 * Tensor to vector. Allow a set of tensors to be flattened to vectors
 * Essentialy internally what ztmu does before output
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "endian.h"
#include "header.h"
#include "matrix.h"
#include "memory.h"
#include "mv.h"
#include "parse.h"
#include "read.h"
#include "utils.h"
#include "write.h"

static const char *name = "zttov";

static void ttov_usage(void)
{
  fprintf(stderr, "%s: usage: %s %s <number of rows per tensor> <in_file> <out_file>\n", name, name, parse_usage());
}

int main(int argc, const char * const argv[])
{
  const char *in;
  const char *out;
  FILE *inp = NULL;
  FILE *outp = NULL;
  u32 prime, nob, noc, nor, len, nod, cols_out, rows_in, rows_out, len_out, i;
  const header *h_in, *h_out;
  word **in_rows, *out_row;

  argv = parse_line(argc, argv, &argc);
  if (4 != argc) {
    ttov_usage();
    exit(1);
  }
  rows_in = strtoul(argv[1], NULL, 0);
  if (0 == rows_in) {
    fprintf(stderr, "%s: cannot have zero input rows, terminating\n", name);
    exit(1);
  }
  in = argv[2];
  out = argv[3];
  memory_init(name, memory);
  endian_init();
  if (0 == open_and_read_binary_header(&inp, &h_in, in, name)) {
    fprintf(stderr, "%s: unable to open %s, terminating\n", name, in);
    exit(1);
  }
  nor = header_get_nor(h_in);
  noc = header_get_noc(h_in);
  nob = header_get_nob(h_in);
  len = header_get_len(h_in);
  prime = header_get_prime(h_in);
  nod = header_get_nod(h_in);
  header_free(h_in);
  if ((nor % rows_in) != 0) {
    fprintf(stderr, "%s: total rows %u from %s is not a multiple of %u, terminating\n", name, nor, in, rows_in);
    exit(1);
  }
  cols_out = rows_in * noc;
  rows_out = nor / rows_in;
  /*
   * Allocate half the memory to the input rows,
   * and the other half to the output row. If the output
   * is very tall and thin, and there's not enough space, too bad
   */
  if (memory_rows(len, 500) < rows_in) {
    fprintf(stderr, "%s: cannot allocate %d rows for %s, terminating\n", name, nor, in);
    fclose(inp);
    exit(1);
  }
  in_rows = matrix_malloc(rows_in);
  for (i = 0; i < rows_in; i++) {
    in_rows[i] = memory_pointer_offset(0, i, len);
  }
  h_out = header_create(prime, nob, nod, cols_out, rows_out);
  len_out = header_get_len(h_out);
  if (memory_rows(len_out, 500) < 1) {
    fprintf(stderr, "%s: cannot allocate %d rows for %s, terminating\n", name, rows_out, out);
    header_free(h_out);
    exit(1);
  }
  out_row = memory_pointer_offset(500, 0, len_out);
  if (0 == open_and_write_binary_header(&outp, h_out, out, name)) {
    fprintf(stderr, "%s: cannot create output for %s, terminating\n", name, out);
    header_free(h_out);
    exit(1);
  }
  /* Loop over rows_out */
  for (i = 0; i < rows_out; i++) {
    if (0 == endian_read_matrix(inp, in_rows, len, rows_in)) {
      fprintf(stderr, "%s: cannot input %d rows for %s, terminating\n", name, nor, in);
      fclose(inp);
      exit(1);
    }
    /* Convert from a tensor back to a vector */
    m_to_v(in_rows, out_row, rows_in, noc, prime);
    if (0 == endian_write_row(outp, out_row, len_out)) {
      fprintf(stderr, "%s: cannot output %d rows for %s, terminating\n", name, rows_out, out);
      header_free(h_out);
      fclose(outp);
      exit(1);
    }
  }
  fclose(inp);
  header_free(h_out);
  fclose(outp);
  memory_dispose();
  return 0;
}
