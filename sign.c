/*
 * $Id: sign.c,v 1.6 2003/02/24 18:02:43 jon Exp $
 *
 * Function compute the orthogonal group sign
 *
 */

#include "sign.h"
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "elements.h"
#include "endian.h"
#include "grease.h"
#include "header.h"
#include "matrix.h"
#include "memory.h"
#include "mul.h"
#include "primes.h"
#include "read.h"
#include "rows.h"
#include "singular.h"
#include "utils.h"
#include "write.h"

int sign(const char *qform, const char *bform, const char *name)
{
  FILE *qinp = NULL, *binp = NULL;
  const header *h_inq, *h_inb;
  unsigned int prime, nob, nor, noc, len, n, **mat;
  unsigned int *sing_row1, *sing_row2, *products, *row, out_num;
  int res;
  grease_struct grease;
  prime_ops prime_operations;
  row_ops row_operations;
  assert(NULL != bform);
  assert(NULL != qform);
  assert(NULL != name);
  if (0 == open_and_read_binary_header(&qinp, &h_inq, qform, name) ||
      0 == open_and_read_binary_header(&binp, &h_inb, bform, name)) {
    if (NULL != qinp) {
      fclose(qinp);
      header_free(h_inq);
    }
    return 1;
  }
  prime = header_get_prime(h_inb);
  if (1 == prime || 1 == header_get_prime(h_inq)) {
    fprintf(stderr, "%s: form cannot be a map, terminating\n", name);
    fclose(qinp);
    header_free(h_inq);
    fclose(binp);
    header_free(h_inb);
    return 1;
  }
  nob = header_get_nob(h_inb);
  nor = header_get_nor(h_inb);
  noc = header_get_noc(h_inb);
  len = header_get_len(h_inb);
  if (noc != nor) {
    fprintf(stderr, "%s: form must be square, terminating\n", name);
    fclose(qinp);
    header_free(h_inq);
    fclose(binp);
    header_free(h_inb);
    return 1;
  }
  header_free(h_inb);
  if (header_get_nob(h_inq) != nob ||
      header_get_noc(h_inq) != noc ||
      header_get_nor(h_inq) != nor ||
      header_get_len(h_inq) != len ||
      header_get_prime(h_inq) != prime) {
    fclose(qinp);
    header_free(h_inq);
    fclose(binp);
  }
  header_free(h_inq);
  if (nor % 2 != 0) {
    fclose(qinp);
    fclose(binp);
    return 0; /* We'll call odd dimension + */
  }
  n = memory_rows(len, 100);
  if (memory_rows(len, 900) < nor + 5 || n < prime) {
    fprintf(stderr, "%s: cannot allocate %d rows, terminating\n",
            name, nor + prime + 5);
    fclose(qinp);
    fclose(binp);
    exit(2);
  }
  (void)grease_level(prime, &grease, n);
  primes_init(prime, &prime_operations);
  rows_init(prime, &row_operations);
  grease_init(&row_operations, &grease);
  if (0 == grease_allocate(prime, len, &grease, 900)){
    fprintf(stderr, "%s: unable to allocate grease, terminating\n", name);
    fclose(qinp);
    fclose(binp);
    return 1;
  }
  /* Now allocate the matrix for the identity plus workspace */
  mat = matrix_malloc(nor + 3);
  for (n = 0; n < nor + 3; n++) {
    mat[n] = memory_pointer_offset(0, n, len);
  }
  sing_row1 = memory_pointer_offset(0, nor + 3, len);
  sing_row2 = memory_pointer_offset(0, nor + 4, len);
  /* Now set up the identity (in reverse order, part of a cunning plan) */
  for (n = 0; n < nor; n++) {
    row_init(mat[n], len);
    put_element_to_row(nob, nor - 1 - n, mat[n], 1);
  }
  products = my_malloc(nor * sizeof(unsigned int));
  while (nor > 2) {
    int start_pos = -1;
    res = singular_vector(mat, mat + noc, sing_row1, &out_num, qinp,
                          noc, nor, nob, len, prime, &grease, qform, name);
    if (0 != res) {
      fclose(binp);
      fclose(qinp);
      matrix_free(mat);
      free(products);
      fprintf(stderr, "%s: cannot find a singular vector, terminating\n", name);
      return 1;
    }
    /* TODO: remove mat[out_num] from the set of vectors, remove the clean etc */
    assert(nor > 3);
    row = mat[out_num];
    mat[out_num] = mat[nor - 1];
    mat[nor - 1] = row;
    if (0 == mul_from_store(&sing_row1, &sing_row2, binp, 0, noc, len, nob, 1, noc, prime,
                            &grease, 0, bform, name)) {
      fclose(binp);
      fclose(qinp);
      matrix_free(mat);
      free(products);
      return 1;
    }
    for (n = 0; n < len; n++) {
      if (0 != sing_row2[n]) {
        start_pos = n;
        break;
      }
    }
    assert(start_pos >= 0);
    for (n = 0; n + 1 < nor; n++) {
      products[n] = (*row_operations.product)(mat[n] + start_pos, sing_row2 + start_pos, len - start_pos);
    }
    n = 0;
    res = -1;
    while (n < nor) {
      if (0 != products[n]) {
        unsigned int elt = products[n];
        if (res < 0) {
          /* First instance, clean the row and remember it */
          if (1 != elt) {
            elt = (*prime_operations.invert)(elt);
            (*row_operations.scaler_in_place)(mat[n], len, elt);
          }
          res = n;
        } else {
          /* Subsequent instance, clean the row with the remembered row */
          elt = (*prime_operations.negate)(elt);
          if (1 == elt) {
            (*row_operations.incer)(mat[res], mat[n], len);
          } else {
            (*row_operations.scaled_incer)(mat[res], mat[n], len, elt);
          }
        }
      }
      n++;
    }
    /* Now remove mat[res] as this isn't a null vector */
    assert(nor > 3);
    row = mat[res];
    mat[res] = mat[nor - 2];
    mat[nor - 2] = row;
    nor -= 2;
  }
  res = singular_vector(mat, mat + noc, sing_row1, &out_num, qinp,
                        noc, nor, nob, len, prime, &grease, qform, name);
  matrix_free(mat);
  free(products);
  fclose(binp);
  fclose(qinp);
  return res;
}
