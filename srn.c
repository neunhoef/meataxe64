/*
 * $Id: srn.c,v 1.4 2001/11/29 01:13:09 jon Exp $: zrn.c,v 1.1 2001/11/12 13:43:38 jon Exp $
 *
 * Simple compute of the rank of a matrix
 *
 */

#include <stdio.h>
#include "endian.h"
#include "memory.h"
#include "clean.h"
#include "header.h"
#include "matrix.h"
#include "read.h"

static const char *name = "srn";

static void rn_usage(void)
{
  fprintf(stderr, "%s: usage: %s <in_file> [<memory>]\n", name, name);
}

int main(int argc, const char * const argv[])
{
  unsigned int n;
  unsigned int memory = MEM_SIZE;
  FILE *inp;
  const header *h;
  unsigned int prime, nob, nor, len, **mat;

  if (2 != argc && 3 != argc) {
    rn_usage();
    exit(1);
  }
  endian_init();
  if (3 == argc) {
    memory = strtoul(argv[2], NULL, 0);
  }
  memory_init(name, memory);
  if (0 == open_and_read_binary_header(&inp, &h, argv[1], name)) {
    exit(1);
  }
  prime = header_get_prime(h);
  nob = header_get_nob(h);
  nor = header_get_nor(h);
  len = header_get_len(h);
  if (memory_rows(len, 1000) < nor) {
    fprintf(stderr, "%s: cannot allocate %d rows for %s, terminating\n", name, nor, argv[1]);
    fclose(inp);
    exit(2);
  }
  /* Now read the matrix */
  mat = matrix_malloc(nor);
  for (n = 0; n < nor; n++) {
    mat[n] = memory_pointer_offset(0, n, len);
  }
  if (0 == endian_read_matrix(inp, mat, len, nor)) {
    fprintf(stderr, "%s: cannot read matrix for %s, terminating\n", name, argv[1]);
    fclose(inp);
    exit(1);
  }
  n = simple_echelise(mat, nor, prime, len, nob);
  free(mat);
  fclose(inp);
  printf("%d\n", n);
  memory_dispose();
  return 0;
}
