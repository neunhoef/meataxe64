/*
 * $Id: clean_vectors.c,v 1.9 2005/10/12 18:20:31 jon Exp $
 *
 * Clean one file of vectors with another
 *
 */

#include "clean_vectors.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include "clean.h"
#include "elements.h"
#include "endian.h"
#include "grease.h"
#include "header.h"
#include "matrix.h"
#include "memory.h"
#include "parse.h"
#include "primes.h"
#include "read.h"
#include "utils.h"
#include "write.h"

/* Clean the dirty vectors with the clean vectors, and output the result */
int clean_vectors(const char *echelised, const char *vectors, const char *output, const char *name)
{
  FILE *inp1 = NULL, *inp2 = NULL, *outp = NULL;
  const header *h_in1, *h_in2;
  header *h_out;
  u32 prime, nob, noc, nor2, nor1, len, d, i, j, max_rows;
  word **rows1, **rows2;
  s64 ptr;
  int *map;
  grease_struct grease;
  prime_ops prime_operations;
  row_ops row_operations;
  assert(NULL != echelised);
  assert(NULL != vectors);
  assert(NULL != output);
  assert(NULL != name);
  if (0 == open_and_read_binary_header(&inp1, &h_in1, echelised, name) ||
      0 == open_and_read_binary_header(&inp2, &h_in2, vectors, name)) {
    if (NULL != inp1) {
      fclose(inp1);
      header_free(h_in1);
    }
    return 0;
  }
  prime = header_get_prime(h_in1);
  nob = header_get_nob(h_in1);
  noc = header_get_noc(h_in1);
  nor1 = header_get_nor(h_in1);
  len = header_get_len(h_in1);
  if (0 == is_a_prime_power(prime)) {
    fprintf(stderr, "%s: prime %u from %s is not a prime power, terminating\n",
            name, prime, echelised);
    fclose(inp1);
    header_free(h_in1);
    fclose(inp2);
    header_free(h_in2);
    return 0;
  }
  if (prime != header_get_prime(h_in2) ||
      nob != header_get_nob(h_in2) ||
      noc != header_get_noc(h_in2) ||
      len != header_get_len(h_in2)) {
    fprintf(stderr, "%s: header incompatibility between %s and %s, terminating\n",
            name, echelised, vectors);
    fclose(inp1);
    header_free(h_in1);
    fclose(inp2);
    header_free(h_in2);
    return 0;
  }
  nor2 = header_get_nor(h_in2);
  h_out = header_create(prime, nob, header_get_nod(h_in1), noc, nor2);
  header_free(h_in1);
  header_free(h_in2);
  errno = 0;
  map = my_malloc(nor1 * sizeof(int));
  /* Initialise arithmetic */
  primes_init(prime, &prime_operations);
  rows_init(prime, &row_operations);
  grease_init(&row_operations, &grease);
  /* Work out how many rows we can handle in store */
  max_rows = memory_rows(len, 450);
  if (0 == grease_level(prime, &grease, memory_rows(len, 100))) {
    fprintf(stderr, "%s: failed to get grease for %s, terminating\n",
            name, echelised);
    fclose(inp1);
    fclose(inp2);
    free(map);
    exit(2);
  }
  /* Give up if too few rows available */
  if (max_rows < 2 * (prime + 1)) {
    fprintf(stderr, "%s: failed to get %u rows for %s, terminating\n",
            name, 2 * (prime + 1), echelised);
    fclose(inp1);
    fclose(inp2);
    free(map);
    exit(2);
  }
  max_rows = (max_rows > noc) ? noc : max_rows; /* Can never need more than this */
  /* Set up the pointers to the workspace rows */
  rows1 = matrix_malloc(max_rows);
  rows2 = matrix_malloc(max_rows);
  for (d = 0; d < max_rows; d++) {
    rows1[d] = memory_pointer_offset(0, d, len);
    rows2[d] = memory_pointer_offset(450, d, len);
  }
  if (0 == open_and_write_binary_header(&outp, h_out, output, name)) {
    matrix_free(rows1);
    matrix_free(rows2);
    fclose(inp1);
    fclose(inp2);
    free(map);
    return 0;
  }
  header_free(h_out);
  if (0 == nor1) {
    /* No rows in clean_vectors, so just copy */
    copy_rest(outp, inp2);
    matrix_free(rows1);
    matrix_free(rows2);
    fclose(inp1);
    fclose(inp2);
    fclose(outp);
    free(map);
    return 1;
  }
  ptr = ftello64(inp1);
  fseeko64(inp1, 0, SEEK_SET);
  for (i = 0; i < nor2; i += max_rows) {
    u32 stride2 = (i + max_rows < nor2) ? max_rows : nor2 - i;
    /* TODO: loop reading stuff from inp1 and cleaning with it */
    if (0 == endian_read_matrix(inp2, rows2, len, stride2)) {
      matrix_free(rows1);
      matrix_free(rows2);
      fclose(inp1);
      fclose(inp2);
      fclose(outp);
      free(map);
      return 0;
    }
    fseeko64(inp1, ptr, SEEK_SET);
    for (j = 0; j < nor1; j += max_rows) {
      u32 stride1 = (j + max_rows < nor1) ? max_rows : nor1 - j;
      if (0 == endian_read_matrix(inp1, rows1, len, stride1)) {
        matrix_free(rows1);
        matrix_free(rows2);
        fclose(inp1);
        fclose(inp2);
        fclose(outp);
        free(map);
        return 0;
      }
#if 1 /* Is this code necessary at all? */
      for (d = 0; d < stride1; d++) {
        map[d] = 0;
      }
#else
      memset(map, 0, stride1 * sizeof(int));
#endif
      for (d = 0; d < stride1; d++) {
        u32 pos;
        word elt = first_non_zero(rows1[d], nob, len, &pos);
        if (0 == elt) {
          fprintf(stderr, "%s: %s contains zero vectors, terminating\n", name, echelised);
          matrix_free(rows1);
          matrix_free(rows2);
          fclose(inp1);
          fclose(inp2);
          fclose(outp);
          free(map);
          return 0;
        }
        map[d] = pos;
      }
      clean(&row_operations, rows1, stride1, rows2, stride2, map, NULL, NULL, 0, grease.level, prime, len, nob, 900, 0, 0, verbose, name);
    }
    if (0 == endian_write_matrix(outp, rows2, len, stride2)) {
      matrix_free(rows1);
      matrix_free(rows2);
      fclose(inp1);
      fclose(inp2);
      fclose(outp);
      free(map);
      return 0;
    }
  }
  matrix_free(rows1);
  matrix_free(rows2);
  fclose(inp1);
  fclose(inp2);
  fclose(outp);
  free(map);
  return 1;
}
