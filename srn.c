/*
 * $Id: srn.c,v 1.9 2002/10/14 19:11:51 jon Exp $: zrn.c,v 1.1 2001/11/12 13:43:38 jon Exp $
 *
 * Simple compute of the rank of a matrix
 *
 */

#include <stdio.h>
#include <errno.h>
#include "endian.h"
#include "memory.h"
#include "clean.h"
#include "header.h"
#include "matrix.h"
#include "parse.h"
#include "read.h"

static const char *name = "srn";

static void rn_usage(void)
{
  fprintf(stderr, "%s: usage: %s [-v] [-m <memory>] <in_file>\n", name, name);
}

int main(int argc, const char * const argv[])
{
  unsigned int n;
  FILE *inp;
  const header *h;
  unsigned int prime, nob, nor, len, **mat;

  argv = parse_line(argc, argv, &argc);
  if (2 != argc) {
    rn_usage();
    exit(1);
  }
  endian_init();
  memory_init(name, memory);
  if (0 == open_and_read_binary_header(&inp, &h, argv[1], name)) {
    exit(1);
  }
  prime = header_get_prime(h);
  if (1 == prime) {
    fprintf(stderr, "%s: cannot handle maps, terminating\n", name);
    fclose(inp);
    header_free(h);
    exit(1);
  }
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
  errno = 0;
  if (0 == endian_read_matrix(inp, mat, len, nor)) {
    if ( 0 != errno) {
      perror(name);
    }
    fprintf(stderr, "%s: cannot read matrix for %s, terminating\n", name, argv[1]);
    fclose(inp);
    exit(1);
  }
  n = simple_echelise(mat, nor, prime, len, nob);
  matrix_free(mat);
  fclose(inp);
  printf("%d\n", n);
  memory_dispose();
  return 0;
}
