/*
 * $Id: qs.c,v 1.11 2002/06/28 08:39:16 jon Exp $
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

void quotient(const char *range, const char *gen,
              const char *out, const char *name)
{
  FILE *inp_r = NULL, *inp_g = NULL, *outp = NULL;
  const header *h_in_r, *h_in_g, *h_out;
  unsigned int prime, nob, noc, nor_r, nor_g, nor_o, len, len_o, d, i, j, k, elt, step_r, step_i, prime_g;
  unsigned int **rows1, **rows2;
  int *map_r, *map_g;
  unsigned int *map_o;
  row_ops row_operations;
  grease_struct grease;
  long long pos;
  int in_store;
  assert(NULL != range);
  assert(NULL != gen);
  assert(NULL != out);
  assert(NULL != name);
  if (0 == open_and_read_binary_header(&inp_r, &h_in_r, range, name) ||
      0 == open_and_read_binary_header(&inp_g, &h_in_g, gen, name)) {
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
  noc = header_get_noc(h_in_r);
  len = header_get_len(h_in_r);
  prime_g = header_get_prime(h_in_g);
  if (noc != header_get_noc(h_in_g) ||
      noc != nor_g ||
      ((1 != prime_g) &&
       (prime != prime_g ||
        nob != header_get_nob(h_in_g)))) {
    fprintf(stderr, "%s: incompatible parameters for %s, %s, terminating\n",
            name, range, gen);
    cleanup(inp_r, inp_g, NULL);
    exit(1);
  }
  if (nor_r > noc) {
    fprintf(stderr, "%s: too many rows in %s, %s, terminating\n",
            name, range, gen);
    cleanup(inp_r, inp_g, NULL);
    exit(1);
  }
  if (1 != prime_g) {
    assert(header_get_len(h_in_g) == len);
  }
  nor_o = nor_g - nor_r; /* Number of output rows and columns */
  h_out = header_create(prime, nob, header_get_nod(h_in_r), nor_o, nor_o);
  len_o = header_get_len(h_out);
  header_free(h_in_r);
  header_free(h_in_g);
  assert(len >= len_o);
  step_i = memory_rows(len, 800);
  step_r = memory_rows(len, 100);
  if (0 == step_r) {
    fprintf(stderr, "%s: cannot allocate 1 row for range %s, termainting\n", name, range);
    cleanup(inp_r, inp_g, NULL);
    exit(1);
  }
  if (0 == open_and_write_binary_header(&outp, h_out, out, name)) {
    cleanup(inp_r, inp_g, outp);
    exit(1);
  }
  in_store = (step_i >= nor_g) && (step_r >= nor_r);
  header_free(h_out);
  rows1 = matrix_malloc(step_r); /* range */
  rows2 = matrix_malloc(step_i); /* gen */
  for (d = 0; d < step_r; d++) {
    rows1[d] = memory_pointer_offset(800, d, len);
  }
  for (d = 0; d < step_i; d++) {
    rows2[d] = memory_pointer_offset(0, d, len);
  }
  map_r = my_malloc(nor_r * sizeof(int));
  map_g = my_malloc(nor_g * sizeof(int));
  map_o = my_malloc(nor_o * sizeof(unsigned int));
  memset(map_r, 0, nor_r * sizeof(int));
  memset(map_g, 0, nor_g * sizeof(int));
  /* Now set up the map */
  pos = ftello64(inp_r); /* Where we are in the range */
  for (i = 0; i < nor_r; i += step_r) {
    unsigned int j, stride_i = (i + step_r <= nor_r) ? step_r : nor_r - i;
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
      assert(j < nor_g);
      assert(0 == map_g[j]);
      map_g[j] = 1; /* Record a submodule significant row */
    }
  }
  /* Now set up the output map */
  i = 0;
  for (d = 0; d < nor_g; d++) {
    if (0 == map_g[d]) {
      map_o[i++] = d; /* Which rows of g to read */
    }
  }
  assert(i == nor_o);
  rows_init(prime, &row_operations);
  grease_init(&row_operations, &grease);
  if (0 == grease_level(prime, &grease, memory_rows(len, 100))) {
    fprintf(stderr, "%s: failed to get grease for %s, terminating\n",
            name, range);
    cleanup(inp_r, inp_g, outp);
    exit(1);
  }
  i = 0; /* Counting the rows from g */
  j = 0; /* Counting the significant rows from g */
  while (j < nor_o) {
    /* Read some rows from inp_g into rows2 */
    unsigned int stride_j = (j + step_i <= nor_o) ? step_i : nor_o - j;
    unsigned int *row_o = memory_pointer(900);
    for (d = 0; d < stride_j; d++) {
      while (map_o[j + d] >= i) {
        /* Handle permutation here */
        if (1 == prime_g) {
          errno = 0;
          if (0 == endian_read_int(&k, inp_g)) {
            if ( 0 != errno) {
              perror(name);
            }
            fprintf(stderr, "%s: failed to read element from %s, terminating\n", name, gen);
            cleanup(inp_r, inp_g, outp);
            exit(1);
          }
          if (map_o[j + d] == i) {
            row_init(rows2[d], len);
            put_element_to_row(nob, k, rows2[d], 1);
          }
        } else {
          errno = 0;
          if (0 == endian_read_row(inp_g, rows2[d], len)) {
            if ( 0 != errno) {
              perror(name);
            }
            fprintf(stderr, "%s: failed to read row from %s, terminating\n", name, gen);
            cleanup(inp_r, inp_g, outp);
            exit(1);
          }
        }
        i++;
        assert(i <= nor_g);
      }
    }
    /* Now loop over inp_r cleaning rows2 */
    if (0 == in_store && 0 != fseeko64(inp_r, pos, SEEK_SET)) {
      fprintf(stderr, "%s: failed to seek in %s, terminating\n",
                name, range);
      cleanup(inp_r, inp_g, outp);
      exit(1);
    }
    for (k = 0; k < nor_r; k += step_r) {
      unsigned int stride_k = (k + step_r <= nor_r) ? step_r : nor_r - k;
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
      for (k = 0; k < nor_o; k++) {
        assert(map_o[k] < noc);
        assert(map_o[k] >= k);
        elt = get_element_from_row(nob, map_o[k], rows2[d]);
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
  free(map_g);
  free(map_o);
  matrix_free(rows1);
  matrix_free(rows2);
}
