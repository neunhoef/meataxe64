/*
 * $Id: qs.c,v 1.3 2001/11/29 01:13:09 jon Exp $
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
#include <string.h>

static void cleanup(FILE *f1, FILE *f2)
{
  if (NULL != f1)
    fclose(f1);
  if (NULL != f2)
    fclose(f2);
}

void quotient(const char *range, const char *gen,
              const char *out, const char *name)
{
  FILE *inp_r = NULL, *inp_g = NULL, *outp = NULL;
  const header *h_in_r, *h_in_g, *h_out;
  unsigned int prime, nob, noc, nor_r, nor_g, nor_o, len, len_o, max_rows, d, i, elt;
  unsigned int **rows1, **rows2, **rows3;
  int *map_r, *map_g;
  unsigned int *map_o;
  row_ops row_operations;
  grease_struct grease;
  assert(NULL != range);
  assert(NULL != gen);
  assert(NULL != out);
  assert(NULL != name);
  if (0 == open_and_read_binary_header(&inp_r, &h_in_r, range, name) ||
      0 == open_and_read_binary_header(&inp_g, &h_in_g, gen, name)) {
    fprintf(stderr, "%s: failed to read header from one of %s, %s, terminating\n",
            name, range, gen);
    cleanup(inp_r, NULL);
    exit(1);
  }
  prime = header_get_prime(h_in_r);
  nob = header_get_nob(h_in_r);
  nor_r = header_get_nor(h_in_r);
  nor_g = header_get_nor(h_in_g);
  noc = header_get_noc(h_in_r);
  len = header_get_len(h_in_r);
  if (noc != header_get_noc(h_in_g) ||
      noc != nor_g ||
      prime != header_get_prime(h_in_g) ||
      nob != header_get_nob(h_in_g)) {
    fprintf(stderr, "%s: incompatible parameters for %s, %s, terminating\n",
            name, range, gen);
    cleanup(inp_r, inp_g);
    exit(1);
  }
  if (nor_r > noc) {
    fprintf(stderr, "%s: too many rows in %s, %s, terminating\n",
            name, range, gen);
    cleanup(inp_r, inp_g);
    exit(1);
  }
  assert(header_get_len(h_in_g) == len);
  nor_o = nor_g - nor_r; /* Number of output rows and columns */
  rows1 = matrix_malloc(nor_g); /* range */
  rows2 = matrix_malloc(nor_g); /* gen */
  rows3 = matrix_malloc(nor_g); /* result */
  max_rows = memory_rows(len, 300);
  if (max_rows < nor_g) {
    fprintf(stderr, "%s: insufficient memory for %s, %s, %s, terminating\n",
            name, range, gen, out);
    cleanup(inp_r, inp_g);
    exit(2);
  }
  for (d = 0; d < nor_g; d++) {
    rows1[d] = memory_pointer_offset(0, d, len);
    rows2[d] = memory_pointer_offset(300, d, len);
    rows3[d] = memory_pointer_offset(600, d, len); /* Really len_o */ 
  }
  if (0 == endian_read_matrix(inp_r, rows1, len, nor_r)) {
    fprintf(stderr, "%s: failed to read rows from %s, terminating\n",
            name, range);
    cleanup(inp_r, inp_g);
    exit(1);
  }
  fclose(inp_r);
  h_out = header_create(prime, nob, header_get_nod(h_in_r), nor_o, nor_o);
  len_o = header_get_len(h_out);
  header_free(h_in_r);
  header_free(h_in_g);
  assert(len >= len_o);
  map_r = my_malloc(nor_r * sizeof(int));
  map_g = my_malloc(nor_g * sizeof(int));
  map_o = my_malloc(nor_o * sizeof(unsigned int));
  memset(map_r, 0, nor_r * sizeof(int));
  memset(map_g, 0, nor_g * sizeof(int));
  for (d = 0; d < nor_r; d++) {
    elt = first_non_zero(rows1[d], nob, len, &i);
    assert(0 != elt);
    NOT_USED(elt);
    map_r[d] = i;
    assert(i < nor_g);
    assert(0 == map_g[i]);
    map_g[i] = 1; /* Record a submodule significant row */
  }
  i = 0;
  for (d = 0; d < nor_g; d++) {
    if (0 == map_g[d]) {
      map_o[i++] = d; /* Which rows of g to read */
    }
  }
  assert(i == nor_o);
  i = 0;
  for (d = 0; d < nor_o; d++) {
    while (map_o[d] >= i) {
      if (0 == endian_read_row(inp_g, rows2[d], len)) {
        fprintf(stderr, "%s: failed to read row from %s, terminating\n", name, gen);
        fclose(inp_g);
        exit(1);
      }
      i++;
      assert(i <= nor_g);
    }
  }
  fclose(inp_g);
  rows_init(prime, &row_operations);
  grease_init(&row_operations, &grease);
  if (0 == grease_level(prime, &grease, memory_rows(len, 100))) {
    fprintf(stderr, "%s: failed to get grease for %s, terminating\n",
            name, range);
    cleanup(inp_r, inp_g);
    exit(1);
  }
  clean(rows1, nor_r, rows2, nor_o, map_r, NULL, NULL, 0,
        grease.level, prime, len, nob, 900, 0, 0, name);
  for (d = 0; d < nor_o; d++) {
    row_init(rows3[d], len_o);
  }
  for (d = 0; d < nor_o; d++) {
    for (i = 0; i < nor_o; i++) {
      assert(map_o[i] < noc);
      elt = get_element_from_row(nob, map_o[i], rows2[d]);
      put_element_to_row(nob, i, rows3[d], elt);
    }
  }
  if (0 == open_and_write_binary_header(&outp, h_out, out, name)) {
    exit(1);
  }
  header_free(h_out);
  if (0 == endian_write_matrix(outp, rows3, len_o, nor_o)) {
    fprintf(stderr, "%s: failed to write output to %s, terminating\n",
            name, out);
    fclose(outp);
    exit(1);
  }
}
