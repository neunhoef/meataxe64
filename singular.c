/*
 * $Id: singular.c,v 1.5 2003/02/24 18:02:43 jon Exp $
 *
 * Function to find a singular vector, given a quadratic form
 *
 */

#include "singular.h"
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "endian.h"
#include "grease.h"
#include "header.h"
#include "matrix.h"
#include "memory.h"
#include "mul.h"
#include "primes.h"
#include "read.h"
#include "span.h"
#include "utils.h"
#include "write.h"

int singular_vector(unsigned int **rows, unsigned int **work,
                    unsigned int *out, unsigned int *out_num, FILE *formp,
                    unsigned int noc, unsigned int nor, unsigned int nob,
                    unsigned int len, unsigned int prime, grease grease,
                    const char *form, const char *name)
{
  unsigned int products[3][3], vector[3];
  unsigned int i, j, power = 1;
  prime_ops prime_operations;
  row_ops row_operations;
  assert(NULL != rows);
  assert(NULL != work);
  assert(NULL != out);
  assert(NULL != form);
  assert(NULL != formp);
  assert(NULL != grease);
  assert(NULL != name);
  assert(NULL != out_num);
  assert(0 != nob);
  assert(0 != noc);
  assert(0 != len);
  assert(0 != prime);
  assert(is_a_prime_power(prime));
  primes_init(prime, &prime_operations);
  rows_init(prime, &row_operations);
  if (nor >= 3) {
    nor = 3;
  }
  if (0 == mul_from_store(rows, work, formp, 0, noc, len, nob, nor, noc, prime,
                          grease, 0, form, name)) {
    return 1;
  }
  for (i = 0; i < nor; i++) {
    for (j = 0; j < nor; j++) {
      products[i][j] = (*row_operations.product)(rows[i], work[j], len);
      /*
      printf("products[%d][%d] = %d\n", i, j, products[i][j]);
      */
    }
  }
#if 0
  for (i = 0; i < nor; i++) {
    unsigned int k, l, m;
    memset(vector, 0, 3 * sizeof(unsigned int));
    vector[i] = 1;
    *out_num = i;
    j = 0;
    while (j < power) {
      unsigned int sub_prod[3], prod = 0, tmp;
      for (l = 0; l < nor; l++) {
        sub_prod[l] = 0;
        for (m = 0; m < nor; m++) {
          tmp = (*prime_operations.mul)(vector[m], products[m][l]);
          sub_prod[l] = (*prime_operations.add)(tmp, sub_prod[l]);
        }
        tmp = (*prime_operations.mul)(sub_prod[l], vector[l]);
        prod = (*prime_operations.add)(prod, tmp);
      }
      if (0 == prod) {
        row_init(out, len);
        for (l = 0; l < nor; l++) {
          if (0 != vector[l]) {
            if (1 != vector[l]) {
              (*row_operations.scaled_incer)(rows[l], out, len, vector[l]);
            } else {
              (*row_operations.incer)(rows[l], out, len);
            }
          }
        }
        return 0;
      }
      /* Now try the next vector in this set */
      k = 0;
      vector[k] += 1;
      while (vector[k] >= prime && k + 1 < nor) {
        vector[k] = 0;
        k += 1;
        vector[k] += 1;
      }
      j++;
    }
    power *= prime;
  }
#else
  memset(vector, 0, 3 * sizeof(unsigned int));
  for (i = 0; i < nor; i++) {
    unsigned int l, m;
    j = 0;
    while (j < power) {
      unsigned int sub_prod[3], prod = 0, tmp;
      span(nor, vector, prime, out_num);
      for (l = 0; l < nor; l++) {
        sub_prod[l] = 0;
        for (m = 0; m < nor; m++) {
          tmp = (*prime_operations.mul)(vector[m], products[m][l]);
          sub_prod[l] = (*prime_operations.add)(tmp, sub_prod[l]);
        }
        tmp = (*prime_operations.mul)(sub_prod[l], vector[l]);
        prod = (*prime_operations.add)(prod, tmp);
      }
      if (0 == prod) {
        row_init(out, len);
        for (l = 0; l < nor; l++) {
          if (0 != vector[l]) {
            if (1 != vector[l]) {
              (*row_operations.scaled_incer)(rows[l], out, len, vector[l]);
            } else {
              (*row_operations.incer)(rows[l], out, len);
            }
          }
        }
        return 0;
      }
      j++;
    }
    power *= prime;
  }
#endif
  return 255;
}

int singular(const char *space, const char *form, const char *out, const char *name)
{
  FILE *inp1, *inp2, *outp;
  const header *h_in1, *h_in2;
  header *h_out;
  unsigned int prime, nob, nor, len, n, **mat;
  unsigned int *out_row, out_num;
  int res;
  grease_struct grease;
  prime_ops prime_operations;
  row_ops row_operations;
  assert(NULL != form);
  assert(NULL != space);
  assert(NULL != out);
  assert(NULL != name);
  if (0 == open_and_read_binary_header(&inp1, &h_in1, space, name) ||
      0 == open_and_read_binary_header(&inp2, &h_in2, form, name)) {
    if (NULL != inp1) {
      fclose(inp1);
      header_free(h_in1);
    }
    return 1;
  }
  prime = header_get_prime(h_in2);
  if (1 == prime) {
    fprintf(stderr, "%s: form cannot be a map, terminating\n", name);
    fclose(inp1);
    header_free(h_in1);
    fclose(inp2);
    header_free(h_in2);
    return 1;
  }
  nob = header_get_nob(h_in2);
  nor = header_get_nor(h_in2);
  len = header_get_len(h_in2);
  if (header_get_noc(h_in2) != nor) {
    fprintf(stderr, "%s: form must be square, terminating\n", name);
    fclose(inp1);
    header_free(h_in1);
    fclose(inp2);
    header_free(h_in2);
    return 1;
  }
  if (header_get_nob(h_in1) != nob ||
      header_get_noc(h_in1) != nor ||
      header_get_len(h_in1) != len ||
      header_get_prime(h_in1) != prime) {
    fclose(inp1);
    header_free(h_in1);
    fclose(inp2);
    header_free(h_in2);
  }
  h_out = header_create(prime, nob, header_get_nod(h_in2), nor, 1);
  n = memory_rows(len, 100);
  header_free(h_in1);
  header_free(h_in2);
  if (memory_rows(len, 900) < 3 + 4 || n < prime) {
    fprintf(stderr, "%s: cannot allocate %d rows for %s and %s, terminating\n",
            name, nor + prime, form, out);
    fclose(inp1);
    fclose(inp2);
    exit(2);
  }
  (void)grease_level(prime, &grease, n);
  primes_init(prime, &prime_operations);
  rows_init(prime, &row_operations);
  grease_init(&row_operations, &grease);
  if (0 == grease_allocate(prime, len, &grease, 900)){
    fprintf(stderr, "%s: unable to allocate grease, terminating\n", name);
    fclose(inp1);
    fclose(inp2);
    return 1;
  }
  /* Now allocate the matrix for the form */
  mat = matrix_malloc(3 + 3);
  for (n = 0; n < 3 + 3; n++) {
    mat[n] = memory_pointer_offset(0, n, len);
  }
  out_row = memory_pointer_offset(0, 3 + 3, len);
  /* Now read at most 3 rows of space */
  errno = 0;
  if (0 == endian_read_matrix(inp1, mat, len, (nor > 3) ? 3 : nor)) {
    if ( 0 != errno) {
      perror(name);
    }
    fclose(inp1);
    fclose(inp2);
    matrix_free(mat);
    return 1;
  }
  fclose(inp1);
  res = singular_vector(mat, mat + 3, out_row, &out_num, inp2,
                        nor, nor, nob, len, prime, &grease, form, name);
  matrix_free(mat);
  fclose(inp2);
  if (0 == res || 255 == res) {
    if (0 == open_and_write_binary_header(&outp, h_out, out, name)) {
      return 1;
    }
    errno = 0;
    if (0 == endian_write_row(outp, out_row, len)) {
      if ( 0 != errno) {
        perror(name);
      }
      fprintf(stderr, "%s: cannot write row to %s, terminating\n", name, out);
      header_free(h_out);
      fclose(outp);
      return 1;
    }
  }
  header_free(h_out);
  fclose(outp);
  return res;
}
