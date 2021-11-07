/*
 * Vector to tensor. Allow a collection of vectors to be put in a form
 * where they can be acted on by a tensor without forming the tensor.
 * Essentialy internally what ztmu does before multiplying
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

static const char *name = "zvtot";

static void vtot_usage(void)
{
  fprintf(stderr, "%s: usage: %s %s <number of columns> <in_file> <out_file>\n", name, name, parse_usage());
}

int main(int argc, const char * const argv[])
{
  const char *in;
  const char *out;
  FILE *inp = NULL;
  FILE *outp = NULL;
  u32 prime, nob, noc, nor, len, nod, cols_out, rows_out, len_out, i;
  const header *h_in, *h_out;
  word *in_row, **out_rows;

  argv = parse_line(argc, argv, &argc);
  if (4 != argc) {
    vtot_usage();
    exit(1);
  }
  in = argv[2];
  cols_out = strtoul(argv[1], NULL, 0);
  out = argv[3];
  if (0 == cols_out) {
    fprintf(stderr, "%s: cannot have zero output columns, terminating\n", name);
    exit(1);
  }
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
  /* Check divisibility, don't want a ragged matrix! */
  if (0 != noc % cols_out) {
    fprintf(stderr, "%s: %" U32_F " does not divide %" U32_F ", terminating\n", name, cols_out, noc);
    fclose(inp);
    exit(1);
  }
  rows_out = noc / cols_out;
  /*
   * Allocate half the memory to the input row,
   * and the other half to the output rows. If the output
   * is very tall and thin, and there's not enough space, too bad
   */
  if (memory_rows(len, 1000) < 2) {
    fprintf(stderr, "%s: cannot allocate row for %s, terminating\n", name, in);
    fclose(inp);
    exit(1);
  }
  h_out = header_create(prime, nob, nod, cols_out, rows_out * nor);
  len_out = header_get_len(h_out);
  out_rows = matrix_malloc(rows_out);
  if (memory_rows(len_out, 500) < rows_out) {
    fprintf(stderr, "%s: cannot allocate %d rows for %s, terminating\n", name, rows_out, out);
    header_free(h_out);
    exit(1);
  }
  for (i = 0; i < rows_out; i++) {
    out_rows[i] = memory_pointer_offset(500, i, len_out);
  }
  if (0 == open_and_write_binary_header(&outp, h_out, out, name)) {
    fprintf(stderr, "%s: cannot create output for %s, terminating\n", name, out);
    header_free(h_out);
    exit(1);
  }
  for (i = 0; i < nor; i++) {
    in_row = memory_pointer_offset(0, 0, len);
    if (0 == endian_read_row(inp, in_row, len)) {
      fprintf(stderr, "%s: unable to read row from %s, terminating\n", name, in);
      fclose(inp);
      exit(1);
    }
    /* Convert to a tensor */
    v_to_m(in_row, out_rows, rows_out, cols_out, prime);
    if (0 == endian_write_matrix(outp, out_rows, len_out, rows_out)) {
      fprintf(stderr, "%s: cannot output %d rows for %s, terminating\n", name, rows_out, out);
      header_free(h_out);
      fclose(outp);
      exit(1);
    }
  }
  header_free(h_out);
  fclose(inp);
  fclose(outp);
  memory_dispose();
  return 0;
}
