/*
 * $Id: powers.c,v 1.4 2002/03/07 13:43:30 jon Exp $
 *
 * Function to compute tensor powers of a matrix, from file
 *
 */

#include "powers.h"
#include "dets.h"
#include "elements.h"
#include "endian.h"
#include "header.h"
#include "matrix.h"
#include "memory.h"
#include "primes.h"
#include "read.h"
#include "rows.h"
#include "utils.h"
#include "write.h"
#include <stdio.h>
#include <assert.h>

static int cleanup(FILE *inp, FILE *outp)
{
  if (NULL != inp)
    fclose(inp);
  if (NULL != outp)
    fclose(outp);
  return 0;
}

int skew_square(const char *m1, const char *m2, const char *name)
{
  FILE *inp = NULL;
  FILE *outp = NULL;
  unsigned int prime, nob, nod, nor_in, len_in, nor_out, len_out;
  const header *h_in = NULL, *h_out = NULL;
  unsigned int i, j, k, l;
  unsigned int **rows, *row_out;
  prime_ops prime_operations;
  assert(NULL != m1);
  assert(NULL != m2);
  assert(NULL != name);
  if (0 == open_and_read_binary_header(&inp, &h_in, m1, name)) {
    if (NULL != h_in) {
      header_free(h_in);
    }
    return cleanup(inp, outp);
  }
  prime = header_get_prime(h_in);
  nob = header_get_nob(h_in);
  nod = header_get_nod(h_in);
  nor_in = header_get_nor(h_in);
  len_in = header_get_len(h_in);
  if (nor_in != header_get_noc(h_in)) {
    fprintf(stderr, "%s: %s is not square, terminating\n", name, m1);
    header_free(h_in);
    return cleanup(inp, NULL);
  }
  header_free(h_in);
  if (nor_in <= 1) {
    fprintf(stderr, "%s: %s has no skew square (dim <= 1), terminating\n", name, m1);
    header_free(h_in);
    return cleanup(inp, NULL);
  }
  if (0 == primes_init(prime, &prime_operations)) {
    fprintf(stderr, "%s: Can't initialise prime operations for %s, terminating\n", name, m1);
    return cleanup(inp, NULL);
  }
  nor_out = (nor_in * (nor_in - 1)) / 2;
  h_out = header_create(prime, nob, nod, nor_out, nor_out);
  len_out = header_get_len(h_out);
  if (memory_rows(len_in, 900) < nor_in ||
      memory_rows(len_out, 100) < 1) {
    fprintf(stderr, "%s: cannot get enough memory, terminating\n", name);
    (void) cleanup(inp, outp);
    exit(2);
  }
  rows = matrix_malloc(nor_in);
  for (i = 0; i < nor_in; i++) {
    rows[i] = memory_pointer_offset(0, i, len_in);
  }
  row_out = memory_pointer(900);
  if (0 == endian_read_matrix(inp, rows, len_in, nor_in)) {
    fprintf(stderr, "%s: cannot read some of %s, terminating\n", name, m1);
    return cleanup(inp, outp);
  }
  if (0 == open_and_write_binary_header(&outp, h_out, m2, name)) {
    return cleanup(inp, outp);
  }
  fclose(inp);
  header_free(h_out);
  for (i = 0; i + 1 < nor_in; i++) {
    /* Down the rows of m1 */
    for (j = i + 1; j < nor_in; j++) {
      /* Down the rows of m1 again */
      unsigned int offset = 0;
      row_init(row_out, len_out);
      for (k = 0; k + 1 < nor_in; k++) {
        /* Along the columns of m1 */
        unsigned int e11 = get_element_from_row(nob, k, rows[i]);
        unsigned int e21 = get_element_from_row(nob, k, rows[j]);
        for (l = k + 1; l < nor_in; l++) {
          /* Along the columns of m1 again */
          unsigned int e12 = get_element_from_row(nob, l, rows[i]);
          unsigned int e22 = get_element_from_row(nob, l, rows[j]);
          unsigned int e = det2(prime_operations, e11, e12, e21, e22);
          put_element_to_row(nob, offset, row_out, e);
          offset++;
        }
      }
      assert(offset == nor_out);
      if (0 == endian_write_row(outp, row_out, len_out)) {
        fprintf(stderr, "%s: cannot write some of %s, terminating\n", name, m2);
        fclose(outp);
        return 0;
      }
    }
  }
  matrix_free(rows);
  fclose(outp);
  return 1;
}

static void make_row(unsigned int nob, unsigned int i, unsigned int j,
                     prime_ops prime_operations, unsigned int *row_out, unsigned int **rows,
                     unsigned int len_out, unsigned int nor_in, unsigned int nor_out)
{
  unsigned int offset = 0;
  unsigned int k, l;
  assert(NULL != rows);
  assert(NULL != row_out);
  assert(0 != nob);
  assert(0 != nor_in);
  assert(0 != nor_out);
  assert(0 != len_out);
  NOT_USED(nor_out);
  row_init(row_out, len_out);
  for (k = 0; k + 1 < nor_in; k++) {
    /* Along the columns of m1 */
    unsigned int elt1 = get_element_from_row(nob, k, rows[i]);
    unsigned int elt2 = get_element_from_row(nob, k, rows[j]);
    for (l = k + 1; l < nor_in; l++) {
      /* Along the columns of m1 again */
      unsigned int elt3 = get_element_from_row(nob, l, rows[i]);
      unsigned int elt4 = get_element_from_row(nob, l, rows[j]);
      unsigned int e1 = (*prime_operations.mul)(elt1, elt4);
      unsigned int e2 = (*prime_operations.negate)((*prime_operations.mul)(elt2, elt3));
      unsigned int e = (*prime_operations.add)(e1, e2);
      put_element_to_row(nob, offset, row_out, e);
      offset++;
    }
  }
  for (k = 0; k < nor_in; k++) {
    /* Along the columns of m1 */
    unsigned int elt1 = get_element_from_row(nob, k, rows[i]);
    unsigned int elt2 = get_element_from_row(nob, k, rows[j]);
    unsigned int elt3 = (*prime_operations.mul)(elt1, elt2);
    put_element_to_row(nob, offset, row_out, elt3);
    offset++;
  }
  assert(offset == nor_out);
}

int sym_square(const char *m1, const char *m2, const char *name)
{
  FILE *inp = NULL;
  FILE *outp = NULL;
  unsigned int prime, nob, nod, nor_in, len_in, nor_out, len_out;
  const header *h_in = NULL, *h_out = NULL;
  unsigned int i, j;
  unsigned int **rows, *row_out;
  prime_ops prime_operations;
  assert(NULL != m1);
  assert(NULL != m2);
  assert(NULL != name);
  if (0 == open_and_read_binary_header(&inp, &h_in, m1, name)) {
    if (NULL != h_in) {
      header_free(h_in);
    }
    return cleanup(inp, outp);
  }
  prime = header_get_prime(h_in);
  nob = header_get_nob(h_in);
  nod = header_get_nod(h_in);
  nor_in = header_get_nor(h_in);
  len_in = header_get_len(h_in);
  if (nor_in != header_get_noc(h_in)) {
    fprintf(stderr, "%s: %s is not square, terminating\n", name, m1);
    header_free(h_in);
    return cleanup(inp, NULL);
  }
  header_free(h_in);
  if (0 == primes_init(prime, &prime_operations)) {
    fprintf(stderr, "%s: Can't initialise prime operations for %s, terminating\n", name, m1);
    return cleanup(inp, NULL);
  }
  nor_out = (nor_in * (nor_in + 1)) / 2;
  h_out = header_create(prime, nob, nod, nor_out, nor_out);
  len_out = header_get_len(h_out);
  if (memory_rows(len_in, 900) < nor_in ||
      memory_rows(len_out, 100) < 1) {
    fprintf(stderr, "%s: cannot get enough memory, terminating\n", name);
    (void) cleanup(inp, outp);
    exit(2);
  }
  rows = matrix_malloc(nor_in);
  for (i = 0; i < nor_in; i++) {
    rows[i] = memory_pointer_offset(0, i, len_in);
  }
  row_out = memory_pointer(900);
  if (0 == endian_read_matrix(inp, rows, len_in, nor_in)) {
    fprintf(stderr, "%s: cannot read some of %s, terminating\n", name, m1);
    return cleanup(inp, outp);
  }
  if (0 == open_and_write_binary_header(&outp, h_out, m2, name)) {
    return cleanup(inp, outp);
  }
  fclose(inp);
  header_free(h_out);
  for (i = 0; i + 1 < nor_in; i++) {
    /* Down the rows of m1 */
    for (j = i + 1; j < nor_in; j++) {
      /* Down the rows of m1 again */
      make_row(nob, i, j, prime_operations, row_out, rows, len_out, nor_in, nor_out);
      if (0 == endian_write_row(outp, row_out, len_out)) {
        fprintf(stderr, "%s: cannot write some of %s, terminating\n", name, m2);
        fclose(outp);
        return 0;
      }
    }
  }
  for (i = 0; i < nor_in; i++) {
    /* Down the rows of m1 */
    make_row(nob, i, i, prime_operations, row_out, rows, len_out, nor_in, nor_out);
    if (0 == endian_write_row(outp, row_out, len_out)) {
      fprintf(stderr, "%s: cannot write some of %s, terminating\n", name, m2);
      fclose(outp);
      return 0;
    }
  }
  matrix_free(rows);
  fclose(outp);
  return 1;
}

int skew_cube(const char *m1, const char *m2, const char *name)
{
  FILE *inp = NULL;
  FILE *outp = NULL;
  unsigned int prime, nob, nod, nor_in, len_in, nor_out, len_out;
  const header *h_in = NULL, *h_out = NULL;
  unsigned int i, j, k, l, m, n;
  unsigned int **rows, *row_out;
  prime_ops prime_operations;
  assert(NULL != m1);
  assert(NULL != m2);
  assert(NULL != name);
  if (0 == open_and_read_binary_header(&inp, &h_in, m1, name)) {
    if (NULL != h_in) {
      header_free(h_in);
    }
    return cleanup(inp, outp);
  }
  prime = header_get_prime(h_in);
  nob = header_get_nob(h_in);
  nod = header_get_nod(h_in);
  nor_in = header_get_nor(h_in);
  len_in = header_get_len(h_in);
  if (nor_in != header_get_noc(h_in)) {
    fprintf(stderr, "%s: %s is not square, terminating\n", name, m1);
    header_free(h_in);
    return cleanup(inp, NULL);
  }
  header_free(h_in);
  if (nor_in <= 2) {
    fprintf(stderr, "%s: %s has no skew cube (dim <= 2), terminating\n", name, m1);
    header_free(h_in);
    return cleanup(inp, NULL);
  }
  if (0 == primes_init(prime, &prime_operations)) {
    fprintf(stderr, "%s: Can't initialise prime operations for %s, terminating\n", name, m1);
    return cleanup(inp, NULL);
  }
  nor_out = (nor_in * (nor_in - 1) * (nor_in - 2)) / 6;
  h_out = header_create(prime, nob, nod, nor_out, nor_out);
  len_out = header_get_len(h_out);
  if (memory_rows(len_in, 900) < nor_in ||
      memory_rows(len_out, 100) < 1) {
    fprintf(stderr, "%s: cannot get enough memory, terminating\n", name);
    (void) cleanup(inp, outp);
    exit(2);
  }
  rows = matrix_malloc(nor_in);
  for (i = 0; i < nor_in; i++) {
    rows[i] = memory_pointer_offset(0, i, len_in);
  }
  row_out = memory_pointer(900);
  if (0 == endian_read_matrix(inp, rows, len_in, nor_in)) {
    fprintf(stderr, "%s: cannot read some of %s, terminating\n", name, m1);
    return cleanup(inp, outp);
  }
  if (0 == open_and_write_binary_header(&outp, h_out, m2, name)) {
    return cleanup(inp, outp);
  }
  fclose(inp);
  header_free(h_out);
  for (i = 0; i + 2 < nor_in; i++) {
    /* Down the rows of m1 */
    for (j = i + 1; j + 1 < nor_in; j++) {
      /* Down the rows of m1 again */
      for (k = j + 1; k < nor_in; k++) {
        /* Down the rows of m1 again */
        unsigned int offset = 0;
        row_init(row_out, len_out);
        for (l = 0; l + 2 < nor_in; l++) {
          /* Along the columns of m1 */
          unsigned e11 = get_element_from_row(nob, l, rows[i]);
          unsigned e21 = get_element_from_row(nob, l, rows[j]);
          unsigned e31 = get_element_from_row(nob, l, rows[k]);
          for (m = l + 1; m + 1 < nor_in; m++) {
            /* Along the columns of m1 again */
            unsigned int e12 = get_element_from_row(nob, m, rows[i]);
            unsigned int e22 = get_element_from_row(nob, m, rows[j]);
            unsigned int e32 = get_element_from_row(nob, m, rows[k]);
            for (n = m + 1; n < nor_in; n++) {
              /* Along the columns of m1 again */
              unsigned int e13 = get_element_from_row(nob, n, rows[i]);
              unsigned int e23 = get_element_from_row(nob, n, rows[j]);
              unsigned int e33 = get_element_from_row(nob, n, rows[k]);
/*
              unsigned int elt = det3(rows, nob, prime_operations, i, l, j, m, k, n);
*/
              unsigned int elt = det3(prime_operations, e11, e12, e13, e21, e22, e23, e31, e32, e33);
              put_element_to_row(nob, offset, row_out, elt);
              offset++;
            }
          }
        }
        assert(offset == nor_out);
        if (0 == endian_write_row(outp, row_out, len_out)) {
          fprintf(stderr, "%s: cannot write some of %s, terminating\n", name, m2);
          fclose(outp);
          return 0;
        }
      }
    }
  }
  matrix_free(rows);
  fclose(outp);
  return 1;
}

int skew_fourth(const char *m1, const char *m2, const char *name)
{
  FILE *inp = NULL;
  FILE *outp = NULL;
  unsigned int prime, nob, nod, nor_in, len_in, nor_out, len_out;
  const header *h_in = NULL, *h_out = NULL;
  unsigned int i, i_1, i_2, i_3, i_4, j_1, j_2, j_3, j_4;
  unsigned int **rows, *row_out, *row1, *row2, *row3, *row4;
  prime_ops prime_operations;
  assert(NULL != m1);
  assert(NULL != m2);
  assert(NULL != name);
  if (0 == open_and_read_binary_header(&inp, &h_in, m1, name)) {
    if (NULL != h_in) {
      header_free(h_in);
    }
    return cleanup(inp, outp);
  }
  prime = header_get_prime(h_in);
  nob = header_get_nob(h_in);
  nod = header_get_nod(h_in);
  nor_in = header_get_nor(h_in);
  len_in = header_get_len(h_in);
  if (nor_in != header_get_noc(h_in)) {
    fprintf(stderr, "%s: %s is not square, terminating\n", name, m1);
    header_free(h_in);
    return cleanup(inp, NULL);
  }
  header_free(h_in);
  if (nor_in <= 3) {
    fprintf(stderr, "%s: %s has no skew fourth (dim <= 3), terminating\n", name, m1);
    header_free(h_in);
    return cleanup(inp, NULL);
  }
  if (0 == primes_init(prime, &prime_operations)) {
    fprintf(stderr, "%s: Can't initialise prime operations for %s, terminating\n", name, m1);
    return cleanup(inp, NULL);
  }
  nor_out = (nor_in * (nor_in - 1) * (nor_in - 2) * (nor_in - 3)) / 24;
  h_out = header_create(prime, nob, nod, nor_out, nor_out);
  len_out = header_get_len(h_out);
  if (memory_rows(len_in, 900) < nor_in ||
      memory_rows(len_out, 100) < 1) {
    fprintf(stderr, "%s: cannot get enough memory, terminating\n", name);
    (void) cleanup(inp, outp);
    exit(2);
  }
  row1 = my_malloc(nor_in * sizeof(unsigned int));
  row2 = my_malloc(nor_in * sizeof(unsigned int));
  row3 = my_malloc(nor_in * sizeof(unsigned int));
  row4 = my_malloc(nor_in * sizeof(unsigned int));
  rows = matrix_malloc(nor_in);
  for (i = 0; i < nor_in; i++) {
    rows[i] = memory_pointer_offset(0, i, len_in);
  }
  row_out = memory_pointer(900);
  if (0 == endian_read_matrix(inp, rows, len_in, nor_in)) {
    fprintf(stderr, "%s: cannot read some of %s, terminating\n", name, m1);
    return cleanup(inp, outp);
  }
  if (0 == open_and_write_binary_header(&outp, h_out, m2, name)) {
    return cleanup(inp, outp);
  }
  fclose(inp);
  header_free(h_out);
  for (i_1 = 0; i_1 + 3 < nor_in; i_1++) {
    /* Down the rows of m1 */
    for (i = 0; i < nor_in; i++) {
      row1[i] = get_element_from_row(nob, i, rows[i_1]);
    }
    for (i_2 = i_1 + 1; i_2 + 2 < nor_in; i_2++) {
      /* Down the rows of m1 again */
      for (i = 0; i < nor_in; i++) {
        row2[i] = get_element_from_row(nob, i, rows[i_2]);
      }
      for (i_3 = i_2 + 1; i_3 + 1 < nor_in; i_3++) {
        /* Down the rows of m1 again */
        for (i = 0; i < nor_in; i++) {
          row3[i] = get_element_from_row(nob, i, rows[i_3]);
        }
        for (i_4 = i_3 + 1; i_4 < nor_in; i_4++) {
          /* Down the rows of m1 again */
          unsigned int offset = 0;
          row_init(row_out, len_out);
          for (i = 0; i < nor_in; i++) {
            row4[i] = get_element_from_row(nob, i, rows[i_4]);
          }
          for (j_1 = 0; j_1 + 3 < nor_in; j_1++) {
            /* Along the columns of m1 */
            unsigned int e11 = row1[j_1];
            unsigned int e21 = row2[j_1];
            unsigned int e31 = row3[j_1];
            unsigned int e41 = row4[j_1];
            for (j_2 = j_1 + 1; j_2 + 2 < nor_in; j_2++) {
              /* Along the columns of m1 again */
              unsigned int e12 = row1[j_2];
              unsigned int e22 = row2[j_2];
              unsigned int e32 = row3[j_2];
              unsigned int e42 = row4[j_2];
              for (j_3 = j_2 + 1; j_3 + 1 < nor_in; j_3++) {
                /* Along the columns of m1 again */
                unsigned int e13 = row1[j_3];
                unsigned int e23 = row2[j_3];
                unsigned int e33 = row3[j_3];
                unsigned int e43 = row4[j_3];
                for (j_4 = j_3 + 1; j_4 < nor_in; j_4++) {
                  /* Along the columns of m1 again */
                  unsigned int e14 = row1[j_4];
                  unsigned int e24 = row2[j_4];
                  unsigned int e34 = row3[j_4];
                  unsigned int e44 = row4[j_4];
                  unsigned int elt = det4(prime_operations, e11, e12, e13, e14,
                                          e21, e22, e23, e24,
                                          e31, e32, e33, e34,
                                          e41, e42, e43, e44);
                  if (0 != elt) {
                    put_element_to_row(nob, offset, row_out, elt);
                  }
                  offset++;
                }
              }
            }
          }
          assert(offset == nor_out);
          if (0 == endian_write_row(outp, row_out, len_out)) {
            fprintf(stderr, "%s: cannot write some of %s, terminating\n", name, m2);
            fclose(outp);
            return 0;
          }
        }
      }
    }
  }
  free(row1);
  free(row2);
  free(row3);
  free(row4);
  matrix_free(rows);
  fclose(outp);
  return 1;
}

int skew_fifth(const char *m1, const char *m2, const char *name)
{
  FILE *inp = NULL;
  FILE *outp = NULL;
  unsigned int prime, nob, nod, nor_in, len_in, nor_out, len_out;
  const header *h_in = NULL, *h_out = NULL;
  unsigned int i, i_1, i_2, i_3, i_4, i_5, j_1, j_2, j_3, j_4, j_5;
  unsigned int **rows, *row_out, *row1, *row2, *row3, *row4, *row5;
  prime_ops prime_operations;
  assert(NULL != m1);
  assert(NULL != m2);
  assert(NULL != name);
  if (0 == open_and_read_binary_header(&inp, &h_in, m1, name)) {
    if (NULL != h_in) {
      header_free(h_in);
    }
    return cleanup(inp, outp);
  }
  prime = header_get_prime(h_in);
  nob = header_get_nob(h_in);
  nod = header_get_nod(h_in);
  nor_in = header_get_nor(h_in);
  len_in = header_get_len(h_in);
  if (nor_in != header_get_noc(h_in)) {
    fprintf(stderr, "%s: %s is not square, terminating\n", name, m1);
    header_free(h_in);
    return cleanup(inp, NULL);
  }
  header_free(h_in);
  if (nor_in <= 4) {
    fprintf(stderr, "%s: %s has no skew fifth (dim <= 4), terminating\n", name, m1);
    header_free(h_in);
    return cleanup(inp, NULL);
  }
  if (0 == primes_init(prime, &prime_operations)) {
    fprintf(stderr, "%s: Can't initialise prime operations for %s, terminating\n", name, m1);
    return cleanup(inp, NULL);
  }
  nor_out = (nor_in * (nor_in - 1) * (nor_in - 2) * (nor_in - 3) * (nor_in - 4)) / 120;
  h_out = header_create(prime, nob, nod, nor_out, nor_out);
  len_out = header_get_len(h_out);
  if (memory_rows(len_in, 900) < nor_in ||
      memory_rows(len_out, 100) < 1) {
    fprintf(stderr, "%s: cannot get enough memory, terminating\n", name);
    (void) cleanup(inp, outp);
    exit(2);
  }
  row1 = my_malloc(nor_in * sizeof(unsigned int));
  row2 = my_malloc(nor_in * sizeof(unsigned int));
  row3 = my_malloc(nor_in * sizeof(unsigned int));
  row4 = my_malloc(nor_in * sizeof(unsigned int));
  row5 = my_malloc(nor_in * sizeof(unsigned int));
  rows = matrix_malloc(nor_in);
  for (i = 0; i < nor_in; i++) {
    rows[i] = memory_pointer_offset(0, i, len_in);
  }
  row_out = memory_pointer(900);
  if (0 == endian_read_matrix(inp, rows, len_in, nor_in)) {
    fprintf(stderr, "%s: cannot read some of %s, terminating\n", name, m1);
    return cleanup(inp, outp);
  }
  if (0 == open_and_write_binary_header(&outp, h_out, m2, name)) {
    return cleanup(inp, outp);
  }
  fclose(inp);
  header_free(h_out);
  for (i_1 = 0; i_1 + 4 < nor_in; i_1++) {
    /* Down the rows of m1 */
    for (i = 0; i < nor_in; i++) {
      row1[i] = get_element_from_row(nob, i, rows[i_1]);
    }
    for (i_2 = i_1 + 1; i_2 + 3 < nor_in; i_2++) {
      /* Down the rows of m1 again */
      for (i = 0; i < nor_in; i++) {
        row2[i] = get_element_from_row(nob, i, rows[i_2]);
      }
      for (i_3 = i_2 + 1; i_3 + 2 < nor_in; i_3++) {
        /* Down the rows of m1 again */
        for (i = 0; i < nor_in; i++) {
          row3[i] = get_element_from_row(nob, i, rows[i_3]);
        }
        for (i_4 = i_3 + 1; i_4 + 1 < nor_in; i_4++) {
          /* Down the rows of m1 again */
          for (i = 0; i < nor_in; i++) {
            row4[i] = get_element_from_row(nob, i, rows[i_4]);
          }
          for (i_5 = i_4 + 1; i_5 < nor_in; i_5++) {
            /* Down the rows of m1 again */
            unsigned int offset = 0;
            for (i = 0; i < nor_in; i++) {
              row5[i] = get_element_from_row(nob, i, rows[i_5]);
            }
            row_init(row_out, len_out);
            for (j_1 = 0; j_1 + 4 < nor_in; j_1++) {
              /* Along the columns of m1 */
              unsigned int e11 = row1[j_1];
              unsigned int e21 = row2[j_1];
              unsigned int e31 = row3[j_1];
              unsigned int e41 = row4[j_1];
              unsigned int e51 = row5[j_1];
              for (j_2 = j_1 + 1; j_2 + 3 < nor_in; j_2++) {
                /* Along the columns of m1 again */
                unsigned int e12 = row1[j_2];
                unsigned int e22 = row2[j_2];
                unsigned int e32 = row3[j_2];
                unsigned int e42 = row4[j_2];
                unsigned int e52 = row5[j_2];
                for (j_3 = j_2 + 1; j_3 + 2 < nor_in; j_3++) {
                  /* Along the columns of m1 again */
                  unsigned int e13 = row1[j_3];
                  unsigned int e23 = row2[j_3];
                  unsigned int e33 = row3[j_3];
                  unsigned int e43 = row4[j_3];
                  unsigned int e53 = row5[j_3];
                  for (j_4 = j_3 + 1; j_4 + 1 < nor_in; j_4++) {
                    /* Along the columns of m1 again */
                    unsigned int e14 = row1[j_4];
                    unsigned int e24 = row2[j_4];
                    unsigned int e34 = row3[j_4];
                    unsigned int e44 = row4[j_4];
                    unsigned int e54 = row5[j_4];
                    for (j_5 = j_4 + 1; j_5 < nor_in; j_5++) {
                      /* Along the columns of m1 again */
                      unsigned int e15 = row1[j_5];
                      unsigned int e25 = row2[j_5];
                      unsigned int e35 = row3[j_5];
                      unsigned int e45 = row4[j_5];
                      unsigned int e55 = row5[j_5];
                      unsigned int elt = det5(prime_operations,
                                              e11, e12, e13, e14, e15,
                                              e21, e22, e23, e24, e25,
                                              e31, e32, e33, e34, e35,
                                              e41, e42, e43, e44, e45,
                                              e51, e52, e53, e54, e55);
                      if (0 != elt) {
                        put_element_to_row(nob, offset, row_out, elt);
                      }
                      offset++;
                    }
                  }
                }
              }
            }
            assert(offset == nor_out);
            if (0 == endian_write_row(outp, row_out, len_out)) {
              fprintf(stderr, "%s: cannot write some of %s, terminating\n", name, m2);
              fclose(outp);
              return 0;
            }
          }
        }
      }
    }
  }
  free(row1);
  free(row2);
  free(row3);
  free(row4);
  free(row5);
  matrix_free(rows);
  fclose(outp);
  return 1;
}

int skew_sixth(const char *m1, const char *m2, const char *name)
{
  FILE *inp = NULL;
  FILE *outp = NULL;
  unsigned int prime, nob, nod, nor_in, len_in, nor_out, len_out;
  const header *h_in = NULL, *h_out = NULL;
  unsigned int i, j, i_1, i_2, i_3, i_4, i_5, i_6, j_1, j_2, j_3, j_4, j_5, j_6;
  unsigned int **rows, *row_out, **int_rows, *row1, *row2, *row3, *row4, *row5, *row6;
  prime_ops prime_operations;
  assert(NULL != m1);
  assert(NULL != m2);
  assert(NULL != name);
  if (0 == open_and_read_binary_header(&inp, &h_in, m1, name)) {
    if (NULL != h_in) {
      header_free(h_in);
    }
    return cleanup(inp, outp);
  }
  prime = header_get_prime(h_in);
  nob = header_get_nob(h_in);
  nod = header_get_nod(h_in);
  nor_in = header_get_nor(h_in);
  len_in = header_get_len(h_in);
  if (nor_in != header_get_noc(h_in)) {
    fprintf(stderr, "%s: %s is not square, terminating\n", name, m1);
    header_free(h_in);
    return cleanup(inp, NULL);
  }
  header_free(h_in);
  if (nor_in <= 5) {
    fprintf(stderr, "%s: %s has no skew sixth (dim <= 5), terminating\n", name, m1);
    header_free(h_in);
    return cleanup(inp, NULL);
  }
  if (0 == primes_init(prime, &prime_operations)) {
    fprintf(stderr, "%s: Can't initialise prime operations for %s, terminating\n", name, m1);
    return cleanup(inp, NULL);
  }
  nor_out = (nor_in * (nor_in - 1) * (nor_in - 2) * (nor_in - 3) * (nor_in - 4) * (nor_in - 5)) / 720;
  h_out = header_create(prime, nob, nod, nor_out, nor_out);
  len_out = header_get_len(h_out);
  if (memory_rows(len_in, 900) < nor_in ||
      memory_rows(len_out, 100) < 1) {
    fprintf(stderr, "%s: cannot get enough memory, terminating\n", name);
    (void) cleanup(inp, outp);
    exit(2);
  }
  rows = matrix_malloc(nor_in);
  int_rows = matrix_malloc(nor_in);
  for (i = 0; i < nor_in; i++) {
    rows[i] = memory_pointer_offset(0, i, len_in);
    int_rows[i] = my_malloc(nor_in * sizeof(unsigned int));
  }
  row_out = memory_pointer(900);
  if (0 == endian_read_matrix(inp, rows, len_in, nor_in)) {
    fprintf(stderr, "%s: cannot read some of %s, terminating\n", name, m1);
    return cleanup(inp, outp);
  }
  for (i = 0; i < nor_in; i++){
    for (j = 0; j < nor_in; j++){
      int_rows[i][j] = get_element_from_row(nob, j, rows[i]);
    }
  }
  if (0 == open_and_write_binary_header(&outp, h_out, m2, name)) {
    return cleanup(inp, outp);
  }
  fclose(inp);
  header_free(h_out);
  for (i_1 = 0; i_1 + 5 < nor_in; i_1++) {
    /* Down the rows of m1 */
    row1 = int_rows[i_1];
    for (i_2 = i_1 + 1; i_2 + 4 < nor_in; i_2++) {
      /* Down the rows of m1 again */
      row2 = int_rows[i_2];
      for (i_3 = i_2 + 1; i_3 + 3 < nor_in; i_3++) {
        /* Down the rows of m1 again */
        row3 = int_rows[i_3];
        for (i_4 = i_3 + 1; i_4 + 2 < nor_in; i_4++) {
          /* Down the rows of m1 again */
          row4 = int_rows[i_4];
          for (i_5 = i_4 + 1; i_5 + 1 < nor_in; i_5++) {
            /* Down the rows of m1 again */
            row5 = int_rows[i_5];
            for (i_6 = i_5 + 1; i_6 < nor_in; i_6++) {
              /* Down the rows of m1 again */
              unsigned int offset = 0;
              row_init(row_out, len_out);
              row6 = int_rows[i_6];
              for (j_1 = 0; j_1 + 5 < nor_in; j_1++) {
                /* Along the columns of m1 */
                unsigned int e11 = row1[j_1];
                unsigned int e21 = row2[j_1];
                unsigned int e31 = row3[j_1];
                unsigned int e41 = row4[j_1];
                unsigned int e51 = row5[j_1];
                unsigned int e61 = row6[j_1];
                for (j_2 = j_1 + 1; j_2 + 4 < nor_in; j_2++) {
                  /* Along the columns of m1 again */
                  unsigned int e12 = row1[j_2];
                  unsigned int e22 = row2[j_2];
                  unsigned int e32 = row3[j_2];
                  unsigned int e42 = row4[j_2];
                  unsigned int e52 = row5[j_2];
                  unsigned int e62 = row6[j_2];
                  for (j_3 = j_2 + 1; j_3 + 3 < nor_in; j_3++) {
                    /* Along the columns of m1 again */
                    unsigned int e13 = row1[j_3];
                    unsigned int e23 = row2[j_3];
                    unsigned int e33 = row3[j_3];
                    unsigned int e43 = row4[j_3];
                    unsigned int e53 = row5[j_3];
                    unsigned int e63 = row6[j_3];
                    for (j_4 = j_3 + 1; j_4 + 2 < nor_in; j_4++) {
                      /* Along the columns of m1 again */
                      unsigned int e14 = row1[j_4];
                      unsigned int e24 = row2[j_4];
                      unsigned int e34 = row3[j_4];
                      unsigned int e44 = row4[j_4];
                      unsigned int e54 = row5[j_4];
                      unsigned int e64 = row6[j_4];
                      for (j_5 = j_4 + 1; j_5 + 1 < nor_in; j_5++) {
                        /* Along the columns of m1 again */
                        unsigned int e15 = row1[j_5];
                        unsigned int e25 = row2[j_5];
                        unsigned int e35 = row3[j_5];
                        unsigned int e45 = row4[j_5];
                        unsigned int e55 = row5[j_5];
                        unsigned int e65 = row6[j_5];
                        for (j_6 = j_5 + 1; j_6 < nor_in; j_6++) {
                          /* Along the columns of m1 again */
                          unsigned int e16 = row1[j_6];
                          unsigned int e26 = row2[j_6];
                          unsigned int e36 = row3[j_6];
                          unsigned int e46 = row4[j_6];
                          unsigned int e56 = row5[j_6];
                          unsigned int e66 = row6[j_6];
                          unsigned int elt = det6(prime_operations,
                                                  e11, e12, e13, e14, e15, e16,
                                                  e21, e22, e23, e24, e25, e26,
                                                  e31, e32, e33, e34, e35, e36,
                                                  e41, e42, e43, e44, e45, e46,
                                                  e51, e52, e53, e54, e55, e56,
                                                  e61, e62, e63, e64, e65, e66);
                          if (0 != elt) {
                            put_element_to_row(nob, offset, row_out, elt);
                          }
                          offset++;
                        }
                      }
                    }
                  }
                }
              }
              assert(offset == nor_out);
              if (0 == endian_write_row(outp, row_out, len_out)) {
                fprintf(stderr, "%s: cannot write some of %s, terminating\n", name, m2);
                fclose(outp);
                return 0;
              }
            }
          }
        }
      }
    }
  }
  for (i = 0; i < nor_in; i++) {
    free(int_rows[i]);
  }
  matrix_free(rows);
  matrix_free(int_rows);
  fclose(outp);
  return 1;
}

int skew_seventh(const char *m1, const char *m2, const char *name)
{
  FILE *inp = NULL;
  FILE *outp = NULL;
  unsigned int prime, nob, nod, nor_in, len_in, nor_out, len_out;
  const header *h_in = NULL, *h_out = NULL;
  unsigned int i, j, i_1, i_2, i_3, i_4, i_5, i_6, i_7, j_1, j_2, j_3, j_4, j_5, j_6, j_7;
  unsigned int **rows, *row_out, **int_rows, *row1, *row2, *row3, *row4, *row5, *row6, *row7;
  prime_ops prime_operations;
  assert(NULL != m1);
  assert(NULL != m2);
  assert(NULL != name);
  if (0 == open_and_read_binary_header(&inp, &h_in, m1, name)) {
    if (NULL != h_in) {
      header_free(h_in);
    }
    return cleanup(inp, outp);
  }
  prime = header_get_prime(h_in);
  nob = header_get_nob(h_in);
  nod = header_get_nod(h_in);
  nor_in = header_get_nor(h_in);
  len_in = header_get_len(h_in);
  if (nor_in != header_get_noc(h_in)) {
    fprintf(stderr, "%s: %s is not square, terminating\n", name, m1);
    header_free(h_in);
    return cleanup(inp, NULL);
  }
  header_free(h_in);
  if (nor_in <= 6) {
    fprintf(stderr, "%s: %s has no skew sixth (dim <= 6), terminating\n", name, m1);
    header_free(h_in);
    return cleanup(inp, NULL);
  }
  if (0 == primes_init(prime, &prime_operations)) {
    fprintf(stderr, "%s: Can't initialise prime operations for %s, terminating\n", name, m1);
    return cleanup(inp, NULL);
  }
  nor_out = (nor_in * (nor_in - 1) * (nor_in - 2) * (nor_in - 3) * (nor_in - 4) * (nor_in - 5) * (nor_in - 6)) / 5040;
  h_out = header_create(prime, nob, nod, nor_out, nor_out);
  len_out = header_get_len(h_out);
  if (memory_rows(len_in, 900) < nor_in ||
      memory_rows(len_out, 100) < 1) {
    fprintf(stderr, "%s: cannot get enough memory, terminating\n", name);
    (void) cleanup(inp, outp);
    exit(2);
  }
  rows = matrix_malloc(nor_in);
  int_rows = matrix_malloc(nor_in);
  for (i = 0; i < nor_in; i++) {
    rows[i] = memory_pointer_offset(0, i, len_in);
    int_rows[i] = my_malloc(nor_in * sizeof(unsigned int));
  }
  row_out = memory_pointer(900);
  if (0 == endian_read_matrix(inp, rows, len_in, nor_in)) {
    fprintf(stderr, "%s: cannot read some of %s, terminating\n", name, m1);
    return cleanup(inp, outp);
  }
  for (i = 0; i < nor_in; i++){
    for (j = 0; j < nor_in; j++){
      int_rows[i][j] = get_element_from_row(nob, j, rows[i]);
    }
  }
  if (0 == open_and_write_binary_header(&outp, h_out, m2, name)) {
    return cleanup(inp, outp);
  }
  fclose(inp);
  header_free(h_out);
  for (i_1 = 0; i_1 + 6 < nor_in; i_1++) {
    /* Down the rows of m1 */
    row1 = int_rows[i_1];
    for (i_2 = i_1 + 1; i_2 + 5 < nor_in; i_2++) {
      /* Down the rows of m1 again */
      row2 = int_rows[i_2];
      for (i_3 = i_2 + 1; i_3 + 4 < nor_in; i_3++) {
        /* Down the rows of m1 again */
        row3 = int_rows[i_3];
        for (i_4 = i_3 + 1; i_4 + 3 < nor_in; i_4++) {
          /* Down the rows of m1 again */
          row4 = int_rows[i_4];
          for (i_5 = i_4 + 1; i_5 + 2 < nor_in; i_5++) {
            /* Down the rows of m1 again */
            row5 = int_rows[i_5];
            for (i_6 = i_5 + 1; i_6 + 1 < nor_in; i_6++) {
              /* Down the rows of m1 again */
              row6 = int_rows[i_6];
              for (i_7 = i_6 + 1; i_7 < nor_in; i_7++) {
                /* Down the rows of m1 again */
                unsigned int offset = 0;
                row_init(row_out, len_out);
                row7 = int_rows[i_7];
                for (j_1 = 0; j_1 + 6 < nor_in; j_1++) {
                  /* Along the columns of m1 */
                  unsigned int e11 = row1[j_1];
                  unsigned int e21 = row2[j_1];
                  unsigned int e31 = row3[j_1];
                  unsigned int e41 = row4[j_1];
                  unsigned int e51 = row5[j_1];
                  unsigned int e61 = row6[j_1];
                  unsigned int e71 = row6[j_1];
                  for (j_2 = j_1 + 1; j_2 + 5 < nor_in; j_2++) {
                    /* Along the columns of m1 again */
                    unsigned int e12 = row1[j_2];
                    unsigned int e22 = row2[j_2];
                    unsigned int e32 = row3[j_2];
                    unsigned int e42 = row4[j_2];
                    unsigned int e52 = row5[j_2];
                    unsigned int e62 = row6[j_2];
                    unsigned int e72 = row7[j_2];
                    for (j_3 = j_2 + 1; j_3 + 4 < nor_in; j_3++) {
                      /* Along the columns of m1 again */
                      unsigned int e13 = row1[j_3];
                      unsigned int e23 = row2[j_3];
                      unsigned int e33 = row3[j_3];
                      unsigned int e43 = row4[j_3];
                      unsigned int e53 = row5[j_3];
                      unsigned int e63 = row6[j_3];
                      unsigned int e73 = row7[j_3];
                      for (j_4 = j_3 + 1; j_4 + 3 < nor_in; j_4++) {
                        /* Along the columns of m1 again */
                        unsigned int e14 = row1[j_4];
                        unsigned int e24 = row2[j_4];
                        unsigned int e34 = row3[j_4];
                        unsigned int e44 = row4[j_4];
                        unsigned int e54 = row5[j_4];
                        unsigned int e64 = row6[j_4];
                        unsigned int e74 = row7[j_4];
                        for (j_5 = j_4 + 1; j_5 + 2 < nor_in; j_5++) {
                          /* Along the columns of m1 again */
                          unsigned int e15 = row1[j_5];
                          unsigned int e25 = row2[j_5];
                          unsigned int e35 = row3[j_5];
                          unsigned int e45 = row4[j_5];
                          unsigned int e55 = row5[j_5];
                          unsigned int e65 = row6[j_5];
                          unsigned int e75 = row6[j_5];
                          for (j_6 = j_5 + 1; j_6 + 1 < nor_in; j_6++) {
                            /* Along the columns of m1 again */
                            unsigned int e16 = row1[j_6];
                            unsigned int e26 = row2[j_6];
                            unsigned int e36 = row3[j_6];
                            unsigned int e46 = row4[j_6];
                            unsigned int e56 = row5[j_6];
                            unsigned int e66 = row6[j_6];
                            unsigned int e76 = row6[j_6];
                            for (j_7 = j_6 + 1; j_7 < nor_in; j_7++) {
                              /* Along the columns of m1 again */
                              unsigned int e17 = row1[j_7];
                              unsigned int e27 = row2[j_7];
                              unsigned int e37 = row3[j_7];
                              unsigned int e47 = row4[j_7];
                              unsigned int e57 = row5[j_7];
                              unsigned int e67 = row6[j_7];
                              unsigned int e77 = row6[j_7];
                              unsigned int elt = det7(prime_operations,
                                                      e11, e12, e13, e14, e15, e16, e17,
                                                      e21, e22, e23, e24, e25, e26, e27,
                                                      e31, e32, e33, e34, e35, e36, e37,
                                                      e41, e42, e43, e44, e45, e46, e47,
                                                      e51, e52, e53, e54, e55, e56, e57,
                                                      e61, e62, e63, e64, e65, e66, e67,
                                                      e71, e72, e73, e74, e75, e76, e77);
                              if (0 != elt) {
                                put_element_to_row(nob, offset, row_out, elt);
                              }
                              offset++;
                            }
                          }
                        }
                      }
                    }
                  }
                }
                assert(offset == nor_out);
                if (0 == endian_write_row(outp, row_out, len_out)) {
                  fprintf(stderr, "%s: cannot write some of %s, terminating\n", name, m2);
                  fclose(outp);
                  return 0;
                }
              }
            }
          }
        }
      }
    }
  }
  for (i = 0; i < nor_in; i++) {
    free(int_rows[i]);
  }
  matrix_free(rows);
  matrix_free(int_rows);
  fclose(outp);
  return 1;
}
