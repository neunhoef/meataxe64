/*
 * $Id: qs.c,v 1.1 2001/11/25 12:44:33 jon Exp $
 *
 * Function to compute quotient space representation
 *
 */

#include "qs.h"
#include "clean.h"
#include "elements.h"
#include "endian.h"
#include "grease.h"
#include "header.h"
#include "matrix.h"
#include "memory.h"
#include "primes.h"
#include "read.h"
#include "rows.h"
#include "utils.h"
#include "write.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#if 0
static void cleanup(FILE *f1, FILE *f2)
{
  if (NULL != f1)
    fclose(f1);
  if (NULL != f2)
    fclose(f2);
}
#endif

void quotient(const char *range, const char *gen,
              const char *out, const char *name)
{
  NOT_USED(range);
  NOT_USED(gen);
  NOT_USED(out);
  NOT_USED(name);
  assert(NULL != range);
  assert(NULL != gen);
  assert(NULL != out);
  assert(NULL != name);
/* TODO: Read range and gen headers, and check compatible
 * TODO: Read range
 * TODO: Create map on columns
 * TODO: Create map of holes
 * TODO: Read hole elements from generator and rearrange
 * TODO: Clean stuff from generator
 * TODO: Contract result and write out
 */
#if 0
  FILE *inp1 = NULL, *inp2 = NULL, *outp = NULL;
  const header *h_in1, *h_in2, *h_out;
  unsigned int prime, nob, noc, nor, len, len_e, max_rows, d, elt;
  unsigned int **rows1, **rows2, **rows3, **rows4;
  int *map;
  prime_ops prime_operations;
  row_ops row_operations;
  grease_struct grease;
  assert(NULL != range);
  assert(NULL != image);
  assert(NULL != out);
  assert(NULL != name);
  if (0 == open_and_read_binary_header(&inp1, &h_in1, range, name) ||
      0 == open_and_read_binary_header(&inp2, &h_in2, image, name)) {
    fprintf(stderr, "%s: failed to read header from one of %s, %s, terminating\n",
            name, range, image);
    cleanup(inp1, NULL);
    exit(1);
  }
  prime = header_get_prime(h_in1);
  nob = header_get_nob(h_in1);
  nor = header_get_nor(h_in1);
  noc = header_get_noc(h_in1);
  len = header_get_len(h_in1);
  if (noc != header_get_noc(h_in2) ||
      nor != header_get_nor(h_in2) ||
      prime != header_get_prime(h_in2) ||
      nob != header_get_nob(h_in2)) {
    fprintf(stderr, "%s: incompatible parameters for %s, %s, terminating\n",
            name, range, image);
    cleanup(inp1, inp2);
    exit(1);
  }
  if (nor > noc) {
    fprintf(stderr, "%s: too many rows in %s, %s, terminating\n",
            name, range, image);
    cleanup(inp1, inp2);
    exit(1);
  }
  h_out = header_create(prime, nob, header_get_nod(h_in1), nor, nor);
  len_e = header_get_len(h_out);
  assert(header_get_len(h_in2) == len);
  assert(len >= len_e);
  primes_init(prime, &prime_operations);
  rows_init(prime, &row_operations);
  grease_init(&row_operations, &grease);
  if (0 == grease_level(prime, &grease, memory_rows(len, 40))) {
    fprintf(stderr, "%s: failed to get grease for %s, terminating\n",
            name, range);
    cleanup(inp1, inp2);
    exit(1);
  }
  rows1 = matrix_malloc(nor); /* range */
  rows2 = matrix_malloc(nor); /* image */
  rows3 = matrix_malloc(nor); /* -1 */
  rows4 = matrix_malloc(nor); /* output */
  max_rows = memory_rows(len, 230);
  if (max_rows < nor) {
    fprintf(stderr, "%s: insufficient memory for %s, %s, %s, terminating\n",
            name, range, image, out);
    cleanup(inp1, inp2);
    exit(1);
  }
  for (d = 0; d < nor; d++) {
    rows1[d] = memory_pointer_offset(0, d, len);
    rows2[d] = memory_pointer_offset(230, d, len);
    rows3[d] = memory_pointer_offset(460, d, len_e);
    rows4[d] = memory_pointer_offset(690, d, len_e);
  }
  if (0 == endian_read_matrix(inp1, rows1, len, nor)) {
    fprintf(stderr, "%s: failed to read rows from %s, terminating\n",
            name, range);
    cleanup(inp1, inp2);
    exit(1);
  }
  fclose(inp1);
  if (0 == endian_read_matrix(inp2, rows2, len, nor)) {
    fprintf(stderr, "%s: failed to read rows from %s, terminating\n",
            name, range);
    fclose(inp2);
    exit(1);
  }
  fclose(inp2);
  map = my_malloc(nor * sizeof(int));
  for (d = 0; d < nor; d++) {
    unsigned int i;
    elt = first_non_zero(rows1[d], nob, len, &i);
    assert(0 != elt);
    NOT_USED(elt);
    map[d] = i;
  }
  elt = (*prime_operations.negate)(1);
  for (d = 0; d < nor; d++) {
    row_init(rows3[d], len_e);
    row_init(rows4[d], len_e);
    put_element_to_row(nob, d, rows3[d], elt);
  }
  clean(rows1, nor, rows2, nor, map, rows3, rows4, 1,
        grease.level, prime, len, nob, 920, 960, len_e, name);
  for (d = 0; d < nor; d++) {
    if (0 == row_is_zero(rows2[d], len)) {
      fprintf(stderr, "Suspicious row %d not cleaned to zero\n", d);
    }
  }
  if (0 == open_and_write_binary_header(&outp, h_out, out, name)) {
    exit(1);
  }
  if (0 == endian_write_matrix(outp, rows4, len_e, nor)) {
    fprintf(stderr, "%s: failed to write output to %s, terminating\n",
            name, out);
    fclose(outp);
    exit(1);
  }
#endif
}
