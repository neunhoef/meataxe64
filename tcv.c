/*
 * $Id: tcv.c,v 1.1 2002/08/27 17:12:38 jon Exp $
 *
 * Function to lift vectors from a tensor condensation representation
 *
 */

#include "tcv.h"
#include <stdio.h>
#include <assert.h>
#include "elements.h"
#include "endian.h"
#include "grease.h"
#include "header.h"
#include "memory.h"
#include "matrix.h"
#include "mul.h"
#include "primes.h"
#include "read.h"
#include "rows.h"
#include "utils.h"
#include "write.h"

static void close_files_and_headers(FILE **q, const header **h_q, unsigned int i)
{
  unsigned int j;
  assert(NULL != q);
  assert(NULL != h_q);
  for (j = 0; j < i; j++) {
    assert(NULL != q[j]);
    fclose(q[j]);
    assert(NULL != h_q[j]);
    header_free(h_q[j]);
  }
}

static int cleanup(unsigned int *left_multiplicities, unsigned int *right_multiplicities,
                   unsigned int *nor_q, unsigned int *noc_q, unsigned int *len_q,
                   FILE *inp,
                   FILE **q, const header **h_q, unsigned int i,
                   const header *h_i,
                   const header *h_o, FILE *outp,
                   unsigned int **rows)
{
  if (NULL != left_multiplicities) {
    free(left_multiplicities);
  }
  if (NULL != right_multiplicities) {
    free(right_multiplicities);
  }
  if (NULL != inp) {
    fclose(inp);
  }
  if (NULL != nor_q) {
    free(nor_q);
  }
  if (NULL != noc_q) {
    free(noc_q);
  }
  if (NULL != len_q) {
    free(len_q);
  }
  if (0 != i) {
    close_files_and_headers(q, h_q, i);
  }
  free(q);
  free(h_q);
  if (NULL != h_i) {
    header_free(h_i);
  }
  if (NULL != inp) {
    fclose(inp);
  }
  if (NULL != h_o) {
    header_free(h_o);
  }
  if (NULL != outp) {
    fclose(outp);
  }
  if (NULL != rows) {
    matrix_free(rows);
  }
  return 0;
}

static int read_numbers(FILE *inp, unsigned int s, unsigned int *out)
{
  unsigned int i;
  assert(NULL != inp);
  assert(0 != s);
  assert(NULL != out);
  for (i = 0; i < s; i++) {
    fscanf(inp, "%u", out + i);
    if (ferror(inp) || feof(inp)) {
      return 0;
    }
  }
  return 1;
}

int lift(unsigned int s, const char *mults_l, const char *mults_r, const char *in,
         const char *out, int argc, const char *const *argv, const char *name)
{
  unsigned int *left_multiplicities = NULL, *right_multiplicities = NULL;
  FILE *inp = NULL, *outp = NULL, **q;
  const header **h_q, *h_i, *h_o;
  unsigned int i, j, nob, nor_i, noc_i, len_i, noc_o, len_o, prime, max_end, max_irr, max_irr_len, extent_q, extent_i, extent_o;
  unsigned int *nor_q = NULL, *noc_q = NULL, *len_q = NULL;
  unsigned int **q_rows;
  unsigned int *row_i, *row_o, *inter_row_i, *inter_row_o;
  grease_struct grease;
  row_ops row_operations;
  assert(0 != s);
  assert(NULL != mults_l);
  assert(NULL != mults_r);
  assert(NULL != in);
  assert(NULL != out);
  assert(NULL != name);
  assert(NULL != argv);
  assert(0 != argc);
  if ((int)s != argc) {
    fprintf(stderr, "%s: incorrect number (%d) of arguments Qi, should be %d, terminating\n", name, argc, s);
    return 0;
  }
  left_multiplicities = my_malloc(s * sizeof(unsigned int));
  right_multiplicities = my_malloc(s * sizeof(unsigned int));
  if (NULL == inp) {
    fprintf(stderr, "%s: failed to open left multiplicities file '%s', terminating\n", name, mults_l);
    return cleanup(left_multiplicities, right_multiplicities,
                   NULL, NULL, NULL, NULL,
                   NULL, NULL, 0, NULL, NULL, NULL, NULL);
  }
  if (0 == read_numbers(inp, s, left_multiplicities)) {
    fprintf(stderr, "%s: failed to read left multiplicities file '%s', terminating\n", name, mults_l);
    return cleanup(left_multiplicities, right_multiplicities,
                   NULL, NULL, NULL, inp,
                   NULL, NULL, 0, NULL, NULL, NULL, NULL);
  }
  fclose(inp);
  inp = fopen(mults_r, "r");
  if (NULL == inp) {
    fprintf(stderr, "%s: failed to open right multiplicities file '%s', terminating\n", name, mults_l);
    return cleanup(left_multiplicities, right_multiplicities,
                   NULL, NULL, NULL, NULL,
                   NULL, NULL, 0, NULL, NULL, NULL, NULL);
  }
  if (0 == read_numbers(inp, s, right_multiplicities)) {
    fprintf(stderr, "%s: failed to read right multiplicities file '%s', terminating\n", name, mults_r);
    return cleanup(left_multiplicities, right_multiplicities,
                   NULL, NULL, NULL, inp,
                   NULL, NULL, 0, NULL, NULL, NULL, NULL);
  }
  fclose(inp);
  if (0 == open_and_read_binary_header(&inp, &h_i, in, name)) {
    return cleanup(left_multiplicities, right_multiplicities,
                   NULL, NULL, NULL, NULL,
                   NULL, NULL, 0, NULL, NULL, NULL, NULL);
  }
  nob = header_get_nob(h_i);
  prime = header_get_prime(h_i);
  nor_i = header_get_nor(h_i);
  noc_i = header_get_noc(h_i);
  len_i = header_get_len(h_i);
  rows_init(prime, &row_operations);
  grease_init(&row_operations, &grease);
  grease.level = 1; /* No point in greasing these little multiplications */
  q = my_malloc(s * sizeof(FILE *));
  h_q = my_malloc(s * sizeof(const header *));
  for (i = 0; i < s; i++) {
    if (0 == open_and_read_binary_header(q + i, h_q + i, argv[i], name)) {
      return cleanup(left_multiplicities, right_multiplicities,
                     NULL, NULL, NULL, inp,
                     q, h_q, i, h_i, NULL, NULL, NULL);
    }
  }
  nor_q = my_malloc(s * sizeof(unsigned int));
  noc_q = my_malloc(s * sizeof(unsigned int));
  len_q = my_malloc(s * sizeof(unsigned int));
  j = 0;
  for (i = 0; i < s; i++) {
    nor_q[i] = header_get_nor(h_q[i]);
    noc_q[i] = header_get_noc(h_q[i]);
    len_q[i] = header_get_len(h_q[i]);
    if (header_get_nob(h_q[i]) != nob || header_get_prime(h_q[i]) != prime) {
      fprintf(stderr, "%s: header incompatibility for '%s', terminating\n", name, argv[i]);
      return cleanup(left_multiplicities, right_multiplicities,
                     nor_q, noc_q, len_q, inp,
                     q, h_q, s, h_i, NULL, NULL, NULL);
    }
    j += left_multiplicities[i] * right_multiplicities[i] * nor_q[i];
#if 0
    /* Pi, Qi checks */
    /* Pi * Qi and Qi * Pi are valid multiplications */
    /* noc(Qi) = dim(Si x Si*) = (dim(Si)) ** 2 */
    /* nor(Qi) = dim((Si x Si*)e) = dim(E(Si, Si)e) */
    if (header_get_prime(h_p[i]) != prime || header_get_prime(h_q[i]) != prime ||
        nor_p[i] != noc_q[i] || noc_p[i] != nor_q[i] ||
        nor_q[i] != dim_end[i] || noc_q[i] != dim_irr[i] * dim_irr[i]) {
      fprintf(stderr, "%s: header incompatibility for '%s' or '%s', terminating\n", name, argv[2 * i], argv[2 * i + 1]);
      return cleanup(left_multiplicities, right_multiplicities,
                     nor_q, noc_q, len_q, inp,
                     q, h_q, s, h_i, NULL, NULL, NULL);
    }
#endif
  }
  if (j != noc_i) {
    fprintf(stderr, "%s: header incompatibility for '%s' and the Qi, terminating\n", name, in);
    return cleanup(left_multiplicities, right_multiplicities,
                   nor_q, noc_q, len_q, inp,
                   q, h_q, s, h_i, NULL, NULL, NULL);
    }
  max_end = 0;
  max_irr = 0;
  for (i = 0; i < s; i++) {
    if (nor_q[i] > max_end) {
      max_end = nor_q[i];
    }
    if (noc_q[i] > max_irr) {
      max_irr = noc_q[i];
    }
  }
  max_irr_len = compute_len(nob, max_irr);
  if (0 == grease_allocate(prime, max_irr_len, &grease, 900)){
    fprintf(stderr, "%s: failed to allocate grease, terminating\n", name);
    return cleanup(left_multiplicities, right_multiplicities,
                   nor_q, noc_q, len_q, inp,
                   q, h_q, s, h_i, NULL, NULL, NULL);
  }
  extent_q = find_extent(s * max_end, max_irr_len);
  extent_i = find_extent(2, len_i);
  q_rows = matrix_malloc(max_end);
  noc_o = 0;
  for (i = 0; i < s; i++) {
    noc_o += left_multiplicities[i] * right_multiplicities[i] * noc_q[i];
  }
  h_o = header_create(prime, nob, header_get_nod(h_i), noc_o, nor_i);
  len_o = header_get_len(h_o);
  extent_o = find_extent(2, len_o);
  if (extent_q + extent_i + extent_o >= 900) {
    fprintf(stderr, "%s: insufficient memory, terminating\n", name);
    (void)cleanup(left_multiplicities, right_multiplicities,
                  nor_q, noc_q, len_q, inp,
                  q, h_q, s, h_i, h_o, outp, NULL);
    return 255;
  }
  for (i = 0; i < max_end; i++) {
    q_rows[i] = memory_pointer_offset(0, i, max_irr_len);
  }
  row_i = memory_pointer(extent_q);
  inter_row_i = memory_pointer_offset(extent_q, 1, len_i);
  row_o = memory_pointer_offset(extent_q + extent_i, 0, len_o);
  inter_row_o = memory_pointer_offset(extent_q + extent_i, 2, len_o);
  for (i = 0; i < s; i++) {
    /* Row loop over distinct irreducibles of H */
    /* Read ahead q[i], only needed once */
    if (0 == endian_read_matrix(q[i], q_rows, len_q[i], nor_q[i])) {
      return cleanup(left_multiplicities, right_multiplicities,
                     nor_q, noc_q, len_q, inp,
                     q, h_q, s, h_i, h_o, outp, q_rows);
    }
  }
  if (0 == open_and_write_binary_header(&outp, h_o, out, name)) {
    return cleanup(left_multiplicities, right_multiplicities,
                   nor_q, noc_q, len_q, inp,
                   q, h_q, s, h_i, h_o, outp, q_rows);
  }
  for (i = 0; i < noc_i; i++) {
    unsigned int col_i = 0, col_o = 0;
    if (0 == endian_read_row(inp, row_i, len_i)) {
      return cleanup(left_multiplicities, right_multiplicities,
                     nor_q, noc_q, len_q, inp,
                     q, h_q, s, h_i, h_o, outp, q_rows);
    }
    row_init(row_o, len_o);
    for (i = 0; i < s; i++) {
      unsigned int k, l = left_multiplicities[i] * right_multiplicities[i];
      for (j = 0; j < l; j++) {
        row_init(inter_row_i, len_i);
        for (k = 0; k < nor_q[i]; k++) {
          unsigned int elt = get_element_from_row(nob, col_i + k, row_i);
          if (0 != elt) {
            put_element_to_row(nob, k, inter_row_i, elt);
          }
        }
        if (0 == mul_in_store(&inter_row_i, q_rows, &inter_row_o,
                              0, 0, nor_q[i], len_q[i],
                              nob, 1, noc_q[i], prime,
                              &grease, in, argv[i], name)) {
          return cleanup(left_multiplicities, right_multiplicities,
                         nor_q, noc_q, len_q, inp,
                         q, h_q, s, h_i, h_o, outp, q_rows);
        }
        for (k = 0; k < noc_q[i]; k++) {
          unsigned int elt = get_element_from_row(nob, k, inter_row_o);
          if (0 != elt) {
            put_element_to_row(nob, col_o + k, row_o, elt);
          }
        }
        col_i += nor_q[i];
        col_o += noc_q[i];
      }
    }
    if (0 == endian_write_row(outp, row_o, len_o)) {
      return cleanup(left_multiplicities, right_multiplicities,
                     nor_q, noc_q, len_q, inp,
                     q, h_q, s, h_i, h_o, outp, q_rows);
    }
  }
  (void)cleanup(left_multiplicities, right_multiplicities,
                nor_q, noc_q, len_q, inp,
                q, h_q, s, h_i, h_o, outp, q_rows);
  return 0;
}
