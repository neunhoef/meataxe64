/*
 * $Id: project.c,v 1.6 2002/06/30 21:33:15 jon Exp $
 *
 * Function to project into quotient space representation
 *
 */

#include "project.h"
#include "clean.h"
#include "elements.h"
#include "endian.h"
#include "grease.h"
#include "header.h"
#include "map_or_row.h"
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
#include <errno.h>

static void cleanup(FILE *f1, FILE *f2, FILE *f3)
{
  if (NULL != f1)
    fclose(f1);
  if (NULL != f2)
    fclose(f2);
  if (NULL != f3)
    fclose(f3);
}

void project(const char *range, const char *in,
             const char *out, const char *name)
{
  FILE *inp_r = NULL, *inp_g = NULL, *outp = NULL;
  const header *h_in_r, *h_in_g, *h_out;
  unsigned int prime, prime_g, nob, noc_r, nor_r, nor_g, nor_o, noc_o, len, len_o, max_rows, d, i, j, k, elt, step, mask, elts_per_word;
  unsigned int **rows1, **rows2;
  int *map_r;
  unsigned int *map_o;
  row_ops row_operations;
  grease_struct grease;
  long long pos;
  int in_store;
  assert(NULL != range);
  assert(NULL != in);
  assert(NULL != out);
  assert(NULL != name);
  if (0 == open_and_read_binary_header(&inp_r, &h_in_r, range, name) ||
      0 == open_and_read_binary_header(&inp_g, &h_in_g, in, name)) {
    cleanup(inp_r, NULL, NULL);
    exit(1);
  }
  prime = header_get_prime(h_in_r);
  if (1 == prime) {
    fprintf(stderr, "%s: cannot handle map as range, terminating\n", name);
    cleanup(inp_r, inp_g, NULL);
    exit(1);
  }
  nob = header_get_nob(h_in_r);
  nor_r = header_get_nor(h_in_r);
  nor_g = header_get_nor(h_in_g);
  noc_r = header_get_noc(h_in_r);
  len = header_get_len(h_in_r);
  prime_g = header_get_prime(h_in_g);
  /* Check cols range = cols g, prime and nob */
  if (noc_r != header_get_noc(h_in_g) ||
      ((1 != prime_g) &&
       (prime != prime_g ||
        nob != header_get_nob(h_in_g)))) {
    fprintf(stderr, "%s: incompatible parameters for %s, %s, terminating\n",
            name, range, in);
    cleanup(inp_r, inp_g, NULL);
    exit(1);
  }
  /* Simple check on range sensibility */
  if (nor_r > noc_r) {
    fprintf(stderr, "%s: too many rows in %s, %s, terminating\n",
            name, range, in);
    cleanup(inp_r, inp_g, NULL);
    exit(1);
  }
  if (1 != prime_g) {
    assert(header_get_len(h_in_g) == len);
  }
  nor_o = nor_g; /* Number of output rows */
  noc_o = noc_r - nor_r; /* Number of output colums */
  h_out = header_create(prime, nob, header_get_nod(h_in_r), noc_o, nor_o);
  len_o = header_get_len(h_out);
  header_free(h_in_r);
  header_free(h_in_g);
  assert(len >= len_o);
  max_rows = memory_rows(len, 450);
  in_store = max_rows >= nor_r;
  step = (0 != in_store) ? nor_r : max_rows;
  if (0 == open_and_write_binary_header(&outp, h_out, out, name)) {
    cleanup(inp_r, inp_g, outp);
    exit(1);
  }
  header_free(h_out);
  rows1 = matrix_malloc(step); /* range */
  rows2 = matrix_malloc(step); /* in */
  for (d = 0; d < step; d++) {
    rows1[d] = memory_pointer_offset(0, d, len);
    rows2[d] = memory_pointer_offset(450, d, len);
  }
  map_r = my_malloc(nor_r * sizeof(int));
  map_o = my_malloc(noc_r * sizeof(int));
  memset(map_r, 0, nor_r * sizeof(int));
  memset(map_o, 0, noc_r * sizeof(int));
  /* Now set up the map */
  pos = ftello64(inp_r); /* Where we are in the range */
  for (i = 0; i < nor_r; i += step) {
    unsigned int j, stride_i = (i + step <= nor_r) ? step : nor_r - i;
    errno = 0;
    if (0 == endian_read_matrix(inp_r, rows1, len, stride_i)) {
      if ( 0 != errno) {
        perror(name);
      }
      fprintf(stderr, "%s: failed to read rows from %s, terminating\n",
              name, range);
      cleanup(inp_r, inp_g, outp);
      exit(1);
    }
    for (d = 0; d < stride_i; d++) {
      elt = first_non_zero(rows1[d], nob, len, &j);
      assert(0 != elt);
      NOT_USED(elt);
      map_r[i + d] = j;
      map_o[j] = 1; /* Mark this column as in the subspace */
    }
  }
  j = 0;
  for (i = 0; i < noc_r; i++) {
    if (0 == map_o[i]) {
      map_o[j] = i;
      j++;
    }
  }
  if (j != noc_o) {
    fprintf(stderr, "%s: rows from %s are dependent, terminating\n",
            name, range);
    cleanup(inp_r, inp_g, outp);
    exit(1);
  }
  rows_init(prime, &row_operations);
  grease_init(&row_operations, &grease);
  if (0 == grease_level(prime, &grease, memory_rows(len, 100))) {
    fprintf(stderr, "%s: failed to get grease for %s, terminating\n",
            name, range);
    cleanup(inp_r, inp_g, outp);
    exit(1);
  }
  mask = get_mask_and_elts(nob, &elts_per_word);
  j = 0; /* Counting the rows from g */
  while (j < nor_o) {
    /* Read some rows from inp_g into rows2 */
    unsigned int stride_j = (j + step <= nor_o) ? step : nor_o - j;
    unsigned int *row_o = memory_pointer(900);
    for (d = 0; d < stride_j; d++) {
      if (0 == read_row(1 == prime_g, inp_g, rows2[d],
                        nob, noc_r, len, d, in, name)) {
        fclose(inp_g);
        exit(1);
      }
    }
    /* Now loop over inp_r cleaning rows2 */
    if (0 == in_store && 0 != fseeko64(inp_r, pos, SEEK_SET)) {
      fprintf(stderr, "%s: failed to seek in %s, terminating\n",
                name, range);
      cleanup(inp_r, inp_g, outp);
      exit(1);
    }
    for (k = 0; k < nor_r; k += step) {
      unsigned int stride_k = (k + step <= nor_r) ? step : nor_r - k;
      errno = 0;
      if (0 == in_store && 0 == endian_read_matrix(inp_r, rows1, len, stride_k)) {
        if ( 0 != errno) {
          perror(name);
        }
        fprintf(stderr, "%s: failed to read rows from %s, terminating\n",
                name, range);
        cleanup(inp_r, inp_g, outp);
        exit(1);
      }
      clean(rows1, stride_k, rows2, stride_j, map_r + k, NULL, NULL, 0,
            grease.level, prime, len, nob, 900, 0, 0, name);
    }
    for (d = 0; d < stride_j; d++) {
      row_init(row_o, len_o);
      for (k = 0; k < noc_o; k++) {
        elt = get_element_from_row_with_params(nob, map_o[k], mask, elts_per_word, rows2[d]);
        put_element_to_row(nob, k, row_o, elt);
      }
      errno = 0;
      if (0 == endian_write_row(outp, row_o, len_o)) {
        if ( 0 != errno) {
          perror(name);
        }
        fprintf(stderr, "%s: failed to write output to %s, terminating\n",
                name, out);
        fclose(outp);
        exit(1);
      }
    }
    j += stride_j;
  }
  fclose(inp_r);
  fclose(inp_g);
  fclose(outp);
  free(map_r);
  free(map_o);
  matrix_free(rows1);
  matrix_free(rows2);
}
