/*
 * $Id: ss.c,v 1.4 2001/12/02 01:35:06 jon Exp $
 *
 * Function to compute subspace representation
 * Will work entirely in RAM if possible, otherwise rereading basis file
 *
 */

#include "ss.h"
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

static void cleanup(FILE *f1, FILE *f2, FILE *f3)
{
  if (NULL != f1)
    fclose(f1);
  if (NULL != f2)
    fclose(f2);
  if (NULL != f3)
    fclose(f3);
}

void subspace(const char *range, const char *image,
              const char *out, const char *name)
{
  FILE *inp1 = NULL, *inp2 = NULL, *outp = NULL;
  const header *h_in1, *h_in2, *h_out;
  unsigned int prime, nob, noc, nor, len, len_e, max_rows, d, elt, step, i;
  unsigned int **rows1, **rows2, **rows3, **rows4;
  int *map;
  prime_ops prime_operations;
  row_ops row_operations;
  grease_struct grease;
  long pos;
  int in_store;
  assert(NULL != range);
  assert(NULL != image);
  assert(NULL != out);
  assert(NULL != name);
  if (0 == open_and_read_binary_header(&inp1, &h_in1, range, name) ||
      0 == open_and_read_binary_header(&inp2, &h_in2, image, name)) {
    fprintf(stderr, "%s: failed to open or read header from one of %s, %s, terminating\n",
            name, range, image);
    cleanup(inp1, inp2, NULL);
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
    cleanup(inp1, inp2, NULL);
    exit(1);
  }
  if (nor > noc) {
    fprintf(stderr, "%s: too many rows in %s, %s, terminating\n",
            name, range, image);
    cleanup(inp1, inp2, NULL);
    exit(1);
  }
  h_out = header_create(prime, nob, header_get_nod(h_in1), nor, nor);
  len_e = header_get_len(h_out);
  assert(header_get_len(h_in2) == len);
  assert(len >= len_e);
  header_free(h_in1);
  header_free(h_in2);
  pos = ftell(inp1); /* Where we are in the range */
  primes_init(prime, &prime_operations);
  rows_init(prime, &row_operations);
  grease_init(&row_operations, &grease);
  if (0 == grease_level(prime, &grease, memory_rows(len, 40))) {
    fprintf(stderr, "%s: failed to get grease for %s, terminating\n",
            name, range);
    cleanup(inp1, inp2, NULL);
    exit(1);
  }
  rows1 = matrix_malloc(nor); /* range */
  rows2 = matrix_malloc(nor); /* image */
  rows3 = matrix_malloc(nor); /* -1 */
  rows4 = matrix_malloc(nor); /* output */
  max_rows = memory_rows(len, 230);
  in_store = max_rows >= nor;
  step = (0 != in_store) ? nor : max_rows;
  if (0 == open_and_write_binary_header(&outp, h_out, out, name)) {
    cleanup(inp1, inp2, outp);
    exit(1);
  }
  for (d = 0; d < step; d++) {
    rows1[d] = memory_pointer_offset(0, d, len);
    rows2[d] = memory_pointer_offset(230, d, len);
    rows3[d] = memory_pointer_offset(460, d, len_e);
    rows4[d] = memory_pointer_offset(690, d, len_e);
  }
  map = my_malloc(nor * sizeof(int));
  for (i = 0; i < nor; i += step) {
    /* Step through image */
    unsigned int j, stride_i = (i + step > nor) ? nor - i : step;
    if (0 == endian_read_matrix(inp2, rows2, len, stride_i)) {
      fprintf(stderr, "%s: failed to read rows from %s, terminating\n",
              name, range);
      cleanup(inp1, inp2, outp);
      exit(1);
    }
    for (d = 0; d < stride_i; d++) {
      row_init(rows4[d], len_e);
    }
    for (j = 0; j < nor; j += step) {
      unsigned int stride_j = (j + step > nor) ? nor - j : step;
      if (0 == endian_read_matrix(inp1, rows1, len, stride_j)) {
        fprintf(stderr, "%s: failed to read rows from %s, terminating\n",
                name, range);
        cleanup(inp1, inp2, outp);
        exit(1);
      }
      if (0 == i) {
        /* First time through, set up map */
        for (d = 0; d < stride_j; d++) {
          unsigned int i;
          elt = first_non_zero(rows1[d], nob, len, &i);
          assert(0 != elt);
          NOT_USED(elt);
          map[d + j] = i;
        }
      }
      elt = (*prime_operations.negate)(1);
      for (d = 0; d < stride_j; d++) {
        row_init(rows3[d], len_e);
        put_element_to_row(nob, d + j, rows3[d], elt);
      }
      clean(rows1, stride_j, rows2, stride_i, map + j, rows3, rows4, 1,
            grease.level, prime, len, nob, 920, 960, len_e, name);
#ifndef NDEBUG
      {
        unsigned int k, elt;
        /* Check for bits not being cleaned, or becoming set again */
        elt = get_element_from_row(nob, map[j], rows2[0]);
        if (0 != elt) {
          fprintf(stderr, "Cleaning with row %d fails to clear bit %d\n", j, map[j]);
        }
        for (k = 0; k < j; k++) {
          elt = get_element_from_row(nob, map[k], rows2[0]);
          if (0 != elt) {
            fprintf(stderr, "Cleaning with row %d has reset bit %d\n", j, map[k]);
          }
        }
      }
#endif
    }
#ifndef NDEBUG
    for (d = 0; d < stride_i; d++) {
      if (0 == row_is_zero(rows2[d], len)) {
        unsigned int k, l;
        elt = first_non_zero(rows2[d], nob, len, &k);
        l = 0;
        while (l < nor) {
          if (map[l] == (int)k) {
            break;
          }
          l++;
        }
        fprintf(stderr, "Suspicious row %d not cleaned to zero, first bit position %d, from row %d\n", d +i, k, l);
      }
    }
#endif
    if (0 == endian_write_matrix(outp, rows4, len_e, stride_i)) {
      fprintf(stderr, "%s: failed to write output to %s, terminating\n",
              name, out);
      cleanup(inp1, inp2, outp);
      exit(1);
    }
    if (0 != fseek(inp1, pos, SEEK_SET)) {
      fprintf(stderr, "%s: failed to seek in %s, terminating\n",
                name, range);
      cleanup(inp1, inp2, outp);
      exit(1);
    }
  }
  fclose(inp1);
  fclose(inp2);
  fclose(outp);
  header_free(h_out);
}
