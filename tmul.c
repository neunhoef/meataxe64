/*
 * $Id: tmul.c,v 1.6 2004/06/08 20:51:07 jon Exp $
 *
 * Function to multiply a matrix by a tensor product
 *
 */

#include "tmul.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "endian.h"
#include "header.h"
#include "matrix.h"
#include "memory.h"
#include "mul.h"
#include "mv.h"
#include "primes.h"
#include "read.h"
#include "rows.h"
#include "tra.h"
#include "utils.h"
#include "write.h"
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

/* Multiply m1 by m2 tensor m3 giving m4. Either of m2 and m3 may be a map */
/* m1 may not be a map */
int tmul(const char *m1, const char *m2, const char *m3,
         const char *m4, const char *name)
{
  FILE *in1 = NULL, *in2 = NULL, *in3 = NULL, *out = NULL;
  const header *h_in, *h1, *h2;
  unsigned int prime, nor, noc, nor1, noc1, nor2, noc2, nob, len1, len2, len, max_len, max_nor, max_rows, i, j;
  unsigned int **rows1, **rows2, *row1, *row2, **ptrs_row1, **ptrs_row2;
  int is_map1, is_map2;
  grease_struct grease;
  prime_ops prime_operations;
  row_ops row_operations;
  assert(NULL != m1);
  assert(NULL != m2);
  assert(NULL != m3);
  assert(NULL != m4);
  assert(NULL != name);
  NOT_USED(j);
  if (0 == open_and_read_binary_header(&in1, &h_in, m1, name) ||
      0 == open_and_read_binary_header(&in2, &h1, m2, name) ||
      0 == open_and_read_binary_header(&in3, &h2, m3, name)) {
    cleanup(in1, in2, in3);
    return 0;
  }
  prime = header_get_prime(h_in);
  is_map1 = 1 == header_get_prime(h1);
  is_map2 = 1 == header_get_prime(h2);
  nob = header_get_nob(h_in);
  noc = header_get_noc(h_in);
  nor = header_get_nor(h_in);
  len = header_get_len(h_in);
  noc1 = header_get_noc(h1);
  nor1 = header_get_nor(h1);
  len1 = header_get_len(h1);
  noc2 = header_get_noc(h2);
  nor2 = header_get_nor(h2);
  len2 = header_get_len(h2);
  if (noc1 != nor1 ||
      noc2 != nor2 ||
      noc1 * noc2 != noc ||
      (prime != header_get_prime(h1) && 0 == is_map1) ||
      (prime != header_get_prime(h2) && 0 == is_map2) ||
      (nob != header_get_nob(h1) && 0 == is_map1) ||
      (nob != header_get_nob(h2) && 0 == is_map2)) {
    fprintf(stderr, "%s: incompatible parameters for %s, %s, %s, terminating\n",
            name, m1, m2, m3);
    cleanup(in1, in2, in3);
    return 0;
  }
  header_free(h1);
  header_free(h2);
  if (is_map1 || is_map2) {
    fprintf(stderr, "%s: cannot handle maps (yet), terminating\n", name);
    cleanup(in1, in2, in3);
    return 0;
  }
  max_nor = (nor1 > nor2) ? nor1 : nor2;
  max_len = (len1 > len2) ? len1 : len2;
  max_rows = memory_rows(max_len, 350);
  /* Check we can fit the tensor operands */
  if (max_rows < max_nor) {
    fprintf(stderr, "%s: cannot get enough memory to hold operands, terminating\n", name);
    cleanup(in1, in2, in3);
    return 0;
  }
  /* Check we can fit a row of operand 1, in its expanded form */
  if (memory_rows(noc1 * len2, 100) < 1) {
    fprintf(stderr, "%s: cannot get enough memory to hold a row of operand 1, terminating\n", name);
    cleanup(in1, in2, in3);
    return 0;
  }
  primes_init(prime, &prime_operations);
  rows_init(prime, &row_operations);
  grease_init(&row_operations, &grease);
  if (0 == grease_level(prime, &grease, memory_rows(max_len, 100))) {
    fprintf(stderr, "%s: failed to get grease for %s, %s, %s, terminating\n",
            name, m1, m2, m3);
    cleanup(in1, in2, in3);
    return 0;
  }
  if (0 == grease_allocate(prime, max_len, &grease, 900)){
    fprintf(stderr, "%s: unable to allocate grease, terminating\n", name);
    cleanup(in1, in2, in3);
    return 0;
  }
  rows1 = matrix_malloc(max_nor);
  rows2 = matrix_malloc(max_nor);
  for (i = 0; i < max_nor; i++) {
    rows1[i] = memory_pointer_offset(0, i, max_len);
    rows2[i] = memory_pointer_offset(350, i, max_len);
  }
  /* Allocate the space for the long rows */
  row1 = memory_pointer(700);
  row2 = memory_pointer(800);
  /* Initialise the points into these two rows */
  ptrs_row1 = matrix_malloc(nor1);
  ptrs_row2 = matrix_malloc(nor1);
  create_pointers(row1, ptrs_row1, nor1, len2, prime);
  create_pointers(row2, ptrs_row2, nor1, len2, prime);
  errno = 0;
  if (0 == endian_read_matrix(in2, rows1, len1, nor1)) {
    if ( 0 != errno) {
      perror(name);
    }
    fprintf(stderr, "%s: failed to read %s, terminating\n",
            name, m2);
    cleanup(in1, in2, in3);
    return 0;
  }
  fclose(in2);
  tra_in_situ(rows1, nor1, nob);
  errno = 0;
  if (0 == endian_read_matrix(in3, rows2, len2, nor2)) {
    if ( 0 != errno) {
      perror(name);
    }
    fprintf(stderr, "%s: failed to read %s, terminating\n",
            name, m3);
    cleanup(in1, NULL, in3);
    return 0;
  }
  fclose(in3);
  if (0 == open_and_write_binary_header(&out, h_in, m4, name)) {
    cleanup(in1, NULL, in3);
    return 0;
  }
  header_free(h_in);
  for (i = 0; i < nor; i++) {
    errno = 0;
    if (0 == endian_read_row(in1, row1, len)) {
      if ( 0 != errno) {
        perror(name);
      }
      fprintf(stderr, "%s: failed to read row %d from %s, terminating\n",
              name, i, m1);
      cleanup(in1, out, NULL);
      return 0;
    }
    v_to_m(row1, ptrs_row2, nor1, nor2, prime);
    if (0 == mul_in_store(ptrs_row2, rows2, ptrs_row1,
                          noc2, len2,
                          nob, nor1, prime,
                          0, &grease)) {
      fprintf(stderr, "%s: failed to multiply using %s, terminating\n",
              name, m3);
      cleanup(in1, out, NULL);
      return 0;
    }
    if (0 == mul_in_store(rows1, ptrs_row1, ptrs_row2,
                          noc1, len2,
                          nob, nor1, prime,
                          0, &grease)) {
      fprintf(stderr, "%s: failed to multiply using %s, terminating\n",
              name, m2);
      cleanup(in1, out, NULL);
      return 0;
    }
    m_to_v(ptrs_row2, row1, nor1, noc2, prime);
    errno = 0;
    if (0 == endian_write_row(out, row1, len)) {
      if ( 0 != errno) {
        perror(name);
      }
      fprintf(stderr, "%s: failed to write output row %d to %s, terminating\n",
              name, i, m4);
      cleanup(in1, out, NULL);
      return 0;
    }
  }
  fclose(in1);
  fclose(out);
  matrix_free(rows1);
  matrix_free(rows2);
  matrix_free(ptrs_row1);
  matrix_free(ptrs_row2);
  grease_free(&grease);
  return 1;
}
