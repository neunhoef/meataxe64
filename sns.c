/*
 * $Id: sns.c,v 1.2 2001/11/19 19:08:49 jon Exp $
 *
 * Simple compute of the null space of a matrix
 *
 */

#include <stdio.h>
#include "endian.h"
#include "elements.h"
#include "memory.h"
#include "clean.h"
#include "header.h"
#include "matrix.h"
#include "read.h"
#include "write.h"

static const char *name = "sns";

static void rn_usage(void)
{
  fprintf(stderr, "%s: usage: %s <in_file> <out_file> [<memory>]\n", name, name);
}

int main(int argc, const char * const argv[])
{
  unsigned int n, rank;
  unsigned int memory = MEM_SIZE;
  FILE *inp, *outp;
  header *h;
  unsigned int prime, nob, nor, len, **mat1, **mat2;

  if (3 != argc && 4 != argc) {
    rn_usage();
    exit(1);
  }
  endian_init();
  if (4 == argc) {
    memory = strtoul(argv[3], NULL, 0);
  }
  memory_init(name, memory);
  inp = fopen(argv[1], "rb");
  if (NULL == inp) {
    fprintf(stderr, "%s: cannot open %s, terminating\n", name, argv[1]);
    exit(1);
  }
  if (0 == read_binary_header(inp, (const header **)&h, argv[1])) {
    fclose(inp);
    exit(1);
  }
  prime = header_get_prime(h);
  nob = header_get_nob(h);
  nor = header_get_nor(h);
  len = header_get_len(h);
  /* TODO: Handle nor != noc here */
  if (memory_rows(len, 500) < nor) {
    fprintf(stderr, "%s: cannot allocate %d rows for %s, terminating\n", name, nor, argv[1]);
    fclose(inp);
    exit(1);
  }
  /* Now read the matrix */
  mat1 = matrix_malloc(nor);
  mat2 = matrix_malloc(nor);
  for (n = 0; n < nor; n++) {
    mat1[n] = memory_pointer_offset(0, n, len);
    mat2[n] = memory_pointer_offset(500, n, len);
  }
  if (0 == endian_read_matrix(inp, mat1, len, nor)) {
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
  outp = fopen(argv[2], "wb");
  if (NULL == outp) {
    fprintf(stderr, "%s: cannot open %s, terminating\n", name, argv[2]);
    exit(1);
  }
  if (0 == write_binary_header(outp, h, argv[2])) {
    fclose(outp);
    exit(1);
  }
  header_set_nor(h, nor - rank);
  for (n = 0; n < nor; n++) {
    if (row_is_zero(mat1[n], len)) {
      (void)endian_write_row(outp, mat2[n], len);
      /* TODO: handle error */
    }
  }
  fclose(outp);
  free(mat1);
  free(mat2);
  printf("%d\n", nor - rank);
  memory_dispose();
  return 0;
}
