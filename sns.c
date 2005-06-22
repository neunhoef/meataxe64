/*
 * $Id: sns.c,v 1.13 2005/06/22 21:52:54 jon Exp $
 *
 * Simple compute of the null space of a matrix
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "clean.h"
#include "endian.h"
#include "elements.h"
#include "header.h"
#include "matrix.h"
#include "memory.h"
#include "parse.h"
#include "read.h"
#include "write.h"

static const char *name = "sns";

static void rn_usage(void)
{
  fprintf(stderr, "%s: usage: %s %s <in_file> <out_file>\n", name, name, parse_usage());
}

int main(int argc, const char * const argv[])
{
  u32 n, rank;
  FILE *inp, *outp;
  const header *h_in, *h_out;
  u32 prime, nob, nor, noc, len;
  word **mat1, **mat2;

  argv = parse_line(argc, argv, &argc);
  if (3 != argc) {
    rn_usage();
    exit(1);
  }
  endian_init();
  memory_init(name, memory);
  if (0 == open_and_read_binary_header(&inp, &h_in, argv[1], name)) {
    exit(1);
  }
  prime = header_get_prime(h_in);
  if (1 == prime) {
    fprintf(stderr, "%s: cannot handle maps, terminating\n", name);
    fclose(inp);
    header_free(h_in);
    exit(1);
  }
  nob = header_get_nob(h_in);
  nor = header_get_nor(h_in);
  noc = header_get_noc(h_in);
  len = header_get_len(h_in);
  if (nor != noc) {
    fprintf(stderr, "%s: cannot handle non-square %s, terminating\n", name, argv[1]);
    fclose(inp);
    exit(1);
  }
  if (memory_rows(len, 500) < nor) {
    fprintf(stderr, "%s: cannot allocate %d rows for %s, terminating\n", name, nor, argv[1]);
    fclose(inp);
    exit(2);
  }
  /* Now read the matrix */
  mat1 = matrix_malloc(nor);
  mat2 = matrix_malloc(nor);
  for (n = 0; n < nor; n++) {
    mat1[n] = memory_pointer_offset(0, n, len);
    mat2[n] = memory_pointer_offset(500, n, len);
  }
  errno = 0;
  if (0 == endian_read_matrix(inp, mat1, len, nor)) {
    if ( 0 != errno) {
      perror(name);
    }
    fprintf(stderr, "%s: cannot read matrix for %s, terminating\n", name, argv[1]);
    fclose(inp);
    exit(1);
  }
  fclose(inp);
  /* Initialise mat2 to identity */
  for (n = 0; n < nor; n++) {
    row_init(mat2[n], len);
    put_element_to_row(nob, n, mat2[n], 1);
  }
  rank = simple_echelise_and_record(mat1, mat2, nor, prime, len, nob);
  /* Now write out the rows of mat2 for which mat1 is zero */
  h_out = header_create(prime, nob, header_get_nod(h_in), noc, nor - rank);
  if (0 == open_and_write_binary_header(&outp, h_out, argv[2], name)) {
    exit(1);
  }
  for (n = 0; n < nor; n++) {
    if (row_is_zero(mat1[n], len)) {
      errno = 0;
      if (0 == endian_write_row(outp, mat2[n], len)) {
        if ( 0 != errno) {
          perror(name);
        }
        fprintf(stderr, "%s: error writing %s, terminating\n", name, argv[2]);
        exit(1);
      }

    }
  }
  fclose(outp);
  matrix_free(mat1);
  matrix_free(mat2);
  printf("%d\n", nor - rank);
  memory_dispose();
  return 0;
}
