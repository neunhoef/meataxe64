/*
 * $Id: tco.c,v 1.9 2003/03/22 22:02:38 jon Exp $
 *
 * Tensor condense one group element
 *
 */

#include "tco.h"
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
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
#include "utils.h"
#include "write.h"

static void close_files_and_headers(FILE **p, FILE **q, const header **h_p, const header **h_q, unsigned int i)
{
  unsigned int j;
  assert(NULL != p);
  assert(NULL != q);
  assert(NULL != h_p);
  assert(NULL != h_q);
  for (j = 0; j < i; j++) {
    assert(NULL != p[j]);
    fclose(p[j]);
    assert(NULL != q[j]);
    fclose(q[j]);
    assert(NULL != h_p[j]);
    header_free(h_p[j]);
    assert(NULL != h_q[j]);
    header_free(h_q[j]);
  }
}

static void free_expanded(unsigned int **expanded, unsigned int nor)
{
  unsigned int i;
  assert(NULL != expanded);
  assert(0 != nor);
  for (i = 0; i < nor; i++) {
    free(expanded[i]);
  }
  matrix_free(expanded);
}

static int cleanup(unsigned int *left_multiplicities, unsigned int *right_multiplicities,
                   unsigned int *dim_irr, unsigned int *dim_end,
                   unsigned int *nor_p, unsigned int *nor_q, unsigned int *noc_p,
                   unsigned int *noc_q, unsigned int *len_p, unsigned int *len_q,
                   FILE *inp, FILE *leftp, FILE *rightp,
                   const header *h_l, const header *h_r,
                   FILE **p, FILE **q, const header **h_p, const header **h_q, unsigned int i,
                   const header *h_o, FILE *outp,
                   unsigned int **rows, unsigned int **lrows, unsigned int **rrows)
{
  if (NULL != left_multiplicities) {
    free(left_multiplicities);
  }
  if (NULL != right_multiplicities) {
    free(right_multiplicities);
  }
  if (NULL != dim_irr) {
    free(dim_irr);
  }
  if (NULL != dim_end) {
    free(dim_end);
  }
  if (NULL != inp) {
    fclose(inp);
  }
  if (NULL != leftp) {
    fclose(leftp);
  }
  if (NULL != rightp) {
    fclose(rightp);
  }
  if (NULL != h_l) {
    header_free(h_l);
  }
  if (NULL != h_r) {
    header_free(h_r);
  }
  if (NULL != nor_p) {
    free(nor_p);
  }
  if (NULL != noc_p) {
    free(noc_p);
  }
  if (NULL != len_p) {
    free(len_p);
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
    close_files_and_headers(p, q, h_p, h_q, i);
  }
  free(p);
  free(q);
  free(h_p);
  free(h_q);
  if (NULL != h_o) {
    header_free(h_o);
  }
  if (NULL != outp) {
    fclose(outp);
  }
  if (NULL != rows) {
    matrix_free(rows);
  }
  if (NULL != lrows) {
    matrix_free(lrows);
  }
  if (NULL != rrows) {
    matrix_free(rrows);
  }
  return 0;
}

int tcondense(unsigned int s, const char *mults_l, const char *mults_r, const char *irr, const char *end,
              const char *left, const char *right, const char *out,
              int argc, const char *const *argv, const char *name)
{
  unsigned int *left_multiplicities = NULL, *right_multiplicities = NULL, *dim_irr = NULL, *dim_end = NULL;
  FILE *inp = NULL, *leftp = NULL, *rightp = NULL, *outp = NULL, **p, **q;
  unsigned int nor, nob, nod, len, prime, nor_l, noc_l, len_l, nor_r, noc_r, len_r,
    max_rows, max_irr, max_irr_len, max_end, max_end_len, i, j, k, l;
  unsigned int *nor_p = NULL, *nor_q = NULL, *noc_p = NULL, *noc_q = NULL, *len_p = NULL, *len_q = NULL;
  const header *h_l, *h_r, *h_o, **h_p, **h_q;
  unsigned int alpha, beta, gamma, delta, extent_l, extent_r, extent_te, extent_end, extent_q, o_r, o_c, m_r, m_c, n_r, n_c;
  unsigned int **rows, **lrows, **rrows, **te_rows, *te_row, **end_rows, **q_rows;
  unsigned int mask, elts_per_word;
  unsigned int **expanded_lrows, **expanded_rrows;
  row_ops row_operations;
  grease_struct grease;
  assert(0 != s);
  assert(NULL != mults_l);
  assert(NULL != mults_r);
  assert(NULL != irr);
  assert(NULL != end);
  assert(NULL != left);
  assert(NULL != right);
  assert(NULL != out);
  assert(NULL != name);
  assert(NULL != argv);
  assert(0 != argc);
  if (2*(int)s != argc) {
    fprintf(stderr, "%s: incorrect number (%d) of arguments Pi, Qi, should be %d, terminating\n", name, argc, 2*s);
    return 0;
  }
  left_multiplicities = my_malloc(s * sizeof(unsigned int));
  right_multiplicities = my_malloc(s * sizeof(unsigned int));
  dim_irr = my_malloc(s * sizeof(unsigned int));
  dim_end = my_malloc(s * sizeof(unsigned int));
  inp = fopen(mults_l, "r");
  if (NULL == inp) {
    fprintf(stderr, "%s: failed to open left multiplicities file '%s', terminating\n", name, mults_l);
    return cleanup(left_multiplicities, right_multiplicities, dim_irr, dim_end,
                   NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
                   NULL, NULL, NULL, NULL, NULL, NULL, 0, NULL, NULL, NULL, NULL, NULL);
  }
  if (0 == read_numbers(inp, s, left_multiplicities)) {
    fprintf(stderr, "%s: failed to read left multiplicities file '%s', terminating\n", name, mults_l);
    return cleanup(left_multiplicities, right_multiplicities, dim_irr, dim_end,
                   NULL, NULL, NULL, NULL, NULL, NULL, inp, NULL, NULL,
                   NULL, NULL, NULL, NULL, NULL, NULL, 0, NULL, NULL, NULL, NULL, NULL);
  }
  fclose(inp);
  inp = fopen(mults_r, "r");
  if (NULL == inp) {
    fprintf(stderr, "%s: failed to open right multiplicities file '%s', terminating\n", name, mults_l);
    return cleanup(left_multiplicities, right_multiplicities, dim_irr, dim_end,
                   NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
                   NULL, NULL, NULL, NULL, NULL, NULL, 0, NULL, NULL, NULL, NULL, NULL);
  }
  if (0 == read_numbers(inp, s, right_multiplicities)) {
    fprintf(stderr, "%s: failed to read right multiplicities file '%s', terminating\n", name, mults_r);
    return cleanup(left_multiplicities, right_multiplicities, dim_irr, dim_end,
                   NULL, NULL, NULL, NULL, NULL, NULL, inp, NULL, NULL,
                   NULL, NULL, NULL, NULL, NULL, NULL, 0, NULL, NULL, NULL, NULL, NULL);
  }
  fclose(inp);
  inp = fopen(irr, "r");
  if (NULL == inp) {
    fprintf(stderr, "%s: failed to open irreducible dimensions file '%s', terminating\n", name, irr);
    return cleanup(left_multiplicities, right_multiplicities, dim_irr, dim_end,
                   NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
                   NULL, NULL, NULL, NULL, NULL, NULL, 0, NULL, NULL, NULL, NULL, NULL);
  }
  if (0 == read_numbers(inp, s, dim_irr)) {
    fprintf(stderr, "%s: failed to read irreducible dimensions file '%s', terminating\n", name, irr);
    return cleanup(left_multiplicities, right_multiplicities, dim_irr, dim_end,
                   NULL, NULL, NULL, NULL, NULL, NULL, inp, NULL, NULL,
                   NULL, NULL, NULL, NULL, NULL, NULL, 0, NULL, NULL, NULL, NULL, NULL);
  }
  fclose(inp);
  inp = fopen(end, "r");
  if (NULL == inp) {
    fprintf(stderr, "%s: failed to open endomorphism ring dimensions file '%s', terminating\n", name, end);
    return cleanup(left_multiplicities, right_multiplicities, dim_irr, dim_end,
                   NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
                   NULL, NULL, NULL, NULL, NULL, NULL, 0, NULL, NULL, NULL, NULL, NULL);
  }
  if (0 == read_numbers(inp, s, dim_end)) {
    fprintf(stderr, "%s: failed to read endomorphism ring dimensions file '%s', terminating\n", name, end);
    return cleanup(left_multiplicities, right_multiplicities, dim_irr, dim_end,
                   NULL, NULL, NULL, NULL, NULL, NULL, inp, NULL, NULL,
                   NULL, NULL, NULL, NULL, NULL, NULL, 0, NULL, NULL, NULL, NULL, NULL);
  }
  fclose(inp);
  /* Now compute the maximum tensor size */
  max_irr = 0;
  max_end = 0;
  for (i = 0; i < s; i++) {
    if (dim_irr[i] > max_irr) {
      max_irr = dim_irr[i];
    }
    if (dim_end[i] > max_end) {
      max_end = dim_end[i];
    }
  }
  nor = 0;
  for (i = 0; i < s; i++) {
    nor += left_multiplicities[i] * right_multiplicities[i] * dim_end[i];
  }
  if (0 == nor) {
    fprintf(stderr, "%s: condensed dimension is zero, terminating\n", name);
    return cleanup(left_multiplicities, right_multiplicities, dim_irr, dim_end,
                   NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
                   NULL, NULL, NULL, NULL, NULL, NULL, 0, NULL, NULL, NULL, NULL, NULL);
  }
  if (0 == open_and_read_binary_header(&leftp, &h_l, left, name)) {
    return cleanup(left_multiplicities, right_multiplicities, dim_irr, dim_end,
                  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
                  NULL, NULL, NULL, NULL, NULL, NULL, 0, NULL, NULL, NULL, NULL, NULL);
    return 0;
  }
  if (0 == open_and_read_binary_header(&rightp, &h_r, right, name)) {
    return cleanup(left_multiplicities, right_multiplicities, dim_irr, dim_end,
                   NULL, NULL, NULL, NULL, NULL, NULL, NULL, leftp, NULL,
                   h_l, NULL, NULL, NULL, NULL, NULL, 0, NULL, NULL, NULL, NULL, NULL);
    return 0;
  }
  nob = header_get_nob(h_l);
  mask = get_mask_and_elts(nob, &elts_per_word);
  nod = header_get_nod(h_l);
  prime = header_get_prime(h_l);
  nor_l = header_get_nor(h_l);
  noc_l = header_get_noc(h_l);
  len_l = header_get_len(h_l);
  nor_r = header_get_nor(h_r);
  noc_r = header_get_noc(h_r);
  len_r = header_get_len(h_r);
  max_irr_len = compute_len(nob, max_irr * max_irr);
  max_end_len = compute_len(nob, max_end);
  if (1 == prime || header_get_prime(h_r) != prime || 0 == is_a_prime_power(prime) ||
      nor_l != noc_l || nor_r != noc_r) {
    fprintf(stderr, "%s: header compatibility failure between '%s' and '%s', terminating\n", name, left, right);
    return cleanup(left_multiplicities, right_multiplicities, dim_irr, dim_end,
                   NULL, NULL, NULL, NULL, NULL, NULL, NULL, leftp, rightp,
                   h_l, h_r, NULL, NULL, NULL, NULL, 0, NULL, NULL, NULL, NULL, NULL);
  }
  p = my_malloc(s * sizeof(FILE *));
  q = my_malloc(s * sizeof(FILE *));
  h_p = my_malloc(s * sizeof(const header *));
  h_q = my_malloc(s * sizeof(const header *));
  for (i = 0; i < s; i++) {
    if (0 == open_and_read_binary_header(q + i, h_q + i, argv[2 * i], name)) {
      return cleanup(left_multiplicities, right_multiplicities, dim_irr, dim_end,
                     NULL, NULL, NULL, NULL, NULL, NULL, NULL, leftp, rightp,
                     h_l, h_r, p, q, h_p, h_q, i, NULL, NULL, NULL, NULL, NULL);
    }
    if (0 == open_and_read_binary_header(p + i, h_p + i, argv[2 * i + 1], name)) {
      fclose(q[i]);
      header_free(h_q[i]);
      return cleanup(left_multiplicities, right_multiplicities, dim_irr, dim_end,
                     NULL, NULL, NULL, NULL, NULL, NULL, NULL, leftp, rightp,
                     h_l, h_r, p, q, h_p, h_q, i, NULL, NULL, NULL, NULL, NULL);
    }
  }
  nor_p = my_malloc(s * sizeof(unsigned int));
  noc_p = my_malloc(s * sizeof(unsigned int));
  len_p = my_malloc(s * sizeof(unsigned int));
  nor_q = my_malloc(s * sizeof(unsigned int));
  noc_q = my_malloc(s * sizeof(unsigned int));
  len_q = my_malloc(s * sizeof(unsigned int));
  for (i = 0; i < s; i++) {
    nor_p[i] = header_get_nor(h_p[i]);
    noc_p[i] = header_get_noc(h_p[i]);
    len_p[i] = header_get_len(h_p[i]);
    nor_q[i] = header_get_nor(h_q[i]);
    noc_q[i] = header_get_noc(h_q[i]);
    len_q[i] = header_get_len(h_q[i]);
    /* Pi, Qi checks */
    /* Pi * Qi and Qi * Pi are valid multiplications */
    /* noc(Qi) = dim(Si x Si*) = (dim(Si)) ** 2 */
    /* nor(Qi) = dim((Si x Si*)e) = dim(E(Si, Si)e) */
    if (header_get_prime(h_p[i]) != prime || header_get_prime(h_q[i]) != prime ||
        nor_p[i] != noc_q[i] || noc_p[i] != nor_q[i] ||
        nor_q[i] != dim_end[i] || noc_q[i] != dim_irr[i] * dim_irr[i]) {
      fprintf(stderr, "%s: header incompatibility for '%s' or '%s', terminating\n", name, argv[2 * i], argv[2 * i + 1]);
      return cleanup(left_multiplicities, right_multiplicities, dim_irr, dim_end,
                     nor_p, noc_p, len_p, nor_q, noc_q, len_q, NULL, leftp, rightp,
                     h_l, h_r, p, q, h_p, h_q, s, NULL, NULL, NULL, NULL, NULL);
    }
  }
  h_o = header_create(prime, nob, nod, nor, nor);
  len = header_get_len(h_o);
  extent_l = find_extent(nor_l, len_l);
  extent_r = find_extent(nor_r, len_r);
  extent_te = find_extent(1 + max_irr * max_irr, max_irr_len);
  extent_end = find_extent(max_end, max_end_len);
  extent_q = find_extent(max_end, max_irr_len);
  if (extent_l + extent_r + extent_te + extent_end + extent_q >= 900) {
    fprintf(stderr, "%s: insufficient memory, terminating\n", name);
    (void)cleanup(left_multiplicities, right_multiplicities, dim_irr, dim_end,
                  nor_p, noc_p, len_p, nor_q, noc_q, len_q, NULL, leftp, rightp,
                  h_l, h_r, p, q, h_p, h_q, s, h_o, NULL, NULL, NULL, NULL);
    return 255;
  }
  lrows = matrix_malloc(nor_l);
  rrows = matrix_malloc(nor_r);
  for (i = 0; i < nor_l; i++) {
    lrows[i] = memory_pointer_offset(0, i, len_l);
  }
  for (i = 0; i < nor_r; i++) {
    rrows[i] = memory_pointer_offset(extent_l, i, len_r);
  }
  if (0 == endian_read_matrix(leftp, lrows, len_l, nor_l)) {
    return cleanup(left_multiplicities, right_multiplicities, dim_irr, dim_end,
                   nor_p, noc_p, len_p, nor_q, noc_q, len_q, NULL, leftp, rightp,
                   h_l, h_r, p, q, h_p, h_q, s, NULL, NULL, NULL, lrows, rrows);
  }
  if (0 == endian_read_matrix(rightp, rrows, len_r, nor_r)) {
    return cleanup(left_multiplicities, right_multiplicities, dim_irr, dim_end,
                   nor_p, noc_p, len_p, nor_q, noc_q, len_q, NULL, leftp, rightp,
                   h_l, h_r, p, q, h_p, h_q, s, NULL, NULL, NULL, lrows, rrows);
  }
  header_free(h_l);
  header_free(h_r);
  fclose(leftp);
  fclose(rightp);
  max_rows = memory_rows(len, 900 - extent_l - extent_r - extent_te - extent_end - extent_q);
  if (max_rows < max_end) {
    fprintf(stderr, "%s: cannot allocate enough rows for output, terminating\n", name);
    (void)cleanup(left_multiplicities, right_multiplicities, dim_irr, dim_end,
                  nor_p, noc_p, len_p, nor_q, noc_q, len_q, NULL, NULL, NULL,
                  NULL, NULL, p, q, h_p, h_q, s, h_o, NULL, NULL, lrows, rrows);
    return 255;
  }
  if (0 == open_and_write_binary_header(&outp, h_o, out, name)) {
    return cleanup(left_multiplicities, right_multiplicities, dim_irr, dim_end,
                   nor_p, noc_p, len_p, nor_q, noc_q, len_q, NULL, NULL, NULL,
                   NULL, NULL, p, q, h_p, h_q, s, h_o, NULL, NULL, lrows, rrows);
  }
  rows = matrix_malloc(max_end);
  for (i = 0; i < max_end; i++) {
    rows[i] = memory_pointer_offset(extent_l + extent_r + extent_te + extent_end + extent_q, i, len);
  }
  te_rows = matrix_malloc(max_irr * max_irr);
  end_rows = matrix_malloc(max_irr * max_irr);
  q_rows = matrix_malloc(max_end);
  /* Rows for the partial tensor */
  for (i = 0; i < max_irr * max_irr; i++) {
    te_rows[i] = memory_pointer_offset(extent_l + extent_r, i, max_irr_len);
    end_rows[i] = memory_pointer_offset(extent_l + extent_r + extent_te, i, max_end_len);
  }
  /* A workspace row for creating partial tensors in */
  te_row = /*memory_pointer_offset(extent_l + extent_r, max_irr * max_irr, max_irr_len)*/
    my_malloc(noc_r * sizeof(unsigned int));
  /* Rows for the product with P */
  for (i = 0; i < max_end; i++) {
    q_rows[i] = memory_pointer_offset(extent_l + extent_r + extent_te + extent_end, i, max_irr_len);
  }
  rows_init(prime, &row_operations);
  grease_init(&row_operations, &grease);
  if (0 == grease_level(prime, &grease, 100)) {
    fprintf(stderr, "%s: failed to determine grease level, terminating\n", name);
    matrix_free(te_rows);
    matrix_free(end_rows);
    matrix_free(q_rows);
    free(te_row);
    return cleanup(left_multiplicities, right_multiplicities, dim_irr, dim_end,
                   nor_p, noc_p, len_p, nor_q, noc_q, len_q, NULL, NULL, NULL,
                   NULL, NULL, p, q, h_p, h_q, s, h_o, NULL, rows, lrows, rrows);
  }
  if (grease.level > 5) {
    grease.level = 5; /* No point in greasing these little multiplications */
  }
  if (0 == grease_allocate(prime, max_irr_len, &grease, 900)){
    fprintf(stderr, "%s: failed to allocate grease, terminating\n", name);
    matrix_free(te_rows);
    matrix_free(end_rows);
    matrix_free(q_rows);
    free(te_row);
    return cleanup(left_multiplicities, right_multiplicities, dim_irr, dim_end,
                   nor_p, noc_p, len_p, nor_q, noc_q, len_q, NULL, NULL, NULL,
                   NULL, NULL, p, q, h_p, h_q, s, h_o, NULL, rows, lrows, rrows);
  }
  expanded_lrows = matrix_malloc(nor_l);
  expanded_rrows = matrix_malloc(nor_r);
  for (i = 0; i < nor_l; i++) {
    expanded_lrows[i] = my_malloc(noc_l * sizeof(unsigned int));
  }
  for (i = 0; i < nor_r; i++) {
    expanded_rrows[i] = my_malloc(noc_r * sizeof(unsigned int));
  }
  for (i = 0; i < nor_l; i++) {
    for (j = 0; j < noc_l; j++) {
      expanded_lrows[i][j] = get_element_from_row_with_params(nob, j, mask, elts_per_word, lrows[i]);
    }
  }
  for (i = 0; i < nor_r; i++) {
    for (j = 0; j < noc_r; j++) {
      expanded_rrows[i][j] = get_element_from_row_with_params(nob, j, mask, elts_per_word, rrows[i]);
    }
  }
  o_r = 0;
  m_r = 0;
  n_r = 0;
  for (i = 0; i < s; i++) {
    /* Row loop over distinct irreducibles of H */
    unsigned int dim_irr_i = dim_irr[i];
    /* Read ahead q[i], only needed once */
    if (0 == endian_read_matrix(q[i], q_rows, len_q[i], nor_q[i])) {
      matrix_free(te_rows);
      matrix_free(end_rows);
      matrix_free(q_rows);
      free_expanded(expanded_lrows, nor_l);
      free_expanded(expanded_rrows, nor_r);
      free(te_row);
      return cleanup(left_multiplicities, right_multiplicities, dim_irr, dim_end,
                     nor_p, noc_p, len_p, nor_q, noc_q, len_q, NULL, NULL, NULL,
                     NULL, NULL, p, q, h_p, h_q, s, h_o, NULL, rows, lrows, rrows);
    }
    for (alpha = 0; alpha < left_multiplicities[i]; alpha++) {
      /* Row loop over multiplicity of Si */
      for (beta = 0; beta < right_multiplicities[i]; beta++) {
        /* Row loop over multiplicity of Si* */
        for (j = 0; j < dim_end[i]; j++) {
          row_init(rows[j], len);
        }
        o_c = 0;
        m_c = 0;
        n_c = 0;
        for (j = 0; j < s; j++) {
          /* Column loop over distinct irreducibles of H */
          unsigned int dim_irr_j = dim_irr[j];
          for (gamma = 0; gamma < left_multiplicities[j]; gamma++) {
            /* Column loop over multiplicity of Sj */
            for (delta = 0; delta < right_multiplicities[j]; delta++) {
              /* Column loop over multiplicity of Sj* */
              unsigned int te_i, te_j, te_k, te_l, te_o_r, te_o_c;
              unsigned int base_i = n_c + delta * dim_irr_j;
              te_o_r = 0;
              for (te_i = 0; te_i < dim_irr_i; te_i++) {
                /* Rows of M */
                unsigned int *expanded_lrow = expanded_lrows[m_r + alpha * dim_irr_i + te_i] + m_c + gamma * dim_irr_j;
                for (te_j = 0; te_j < dim_irr_i; te_j++) {
                  /* Rows of N */
                  unsigned int te_o_c_word = 0;
                  unsigned int te_o_c_offset = 0;
                  unsigned int word = 0;
                  unsigned int *expanded_rrow = expanded_rrows[n_r + beta * dim_irr_i + te_j];
                  row_init(te_rows[te_o_r], max_irr_len);
                  te_o_c = 0;
                  for (te_k = 0; te_k < dim_irr_j; te_k++) {
                    /* Columns of M */
                    unsigned int elt = expanded_lrow[te_k];
                    if (0 != elt) {
                      unsigned int *row = expanded_rrow + base_i; /* was rrows */
                      if (1 != elt) {
                        (*row_operations.scaler)(row, te_row, dim_irr_j, elt);
                        row = te_row;
                      }
                      for (te_l = 0; te_l < dim_irr_j; te_l++) {
                        elt = row[te_l];
                        if (0 != elt) {
                          word |= elt << te_o_c_offset;
                        }
                        te_o_c_offset += nob;
                        if (te_o_c_offset + nob > bits_in_unsigned_int) {
                          te_o_c_offset = 0;
                          te_rows[te_o_r][te_o_c_word] = word;
                          te_o_c_word++;
                          word = 0;
                        }
                      }
                    } else {
                      te_o_c_offset += nob * dim_irr_j;
                      if (te_o_c_offset + nob > bits_in_unsigned_int) {
                        te_rows[te_o_r][te_o_c_word] = word;
                        te_o_c_word += te_o_c_offset / (elts_per_word * nob);
                        te_o_c_offset %= (elts_per_word * nob);
                        word = 0;
                      }
                    }
                    te_o_c += dim_irr_j;
                  }
                  te_rows[te_o_r][te_o_c_word] = word;
                  te_o_r++;
                }
              }
              if (0 == mul_from_store(te_rows, end_rows, p[j], 0/*is_map*/, dim_irr_j * dim_irr_j,
                                      len_p[j], nob, dim_irr_i * dim_irr_i, dim_end[j], prime,
                                      &grease, 0, argv[2 * j + 1], name)) {
                matrix_free(te_rows);
                matrix_free(end_rows);
                matrix_free(q_rows);
                grease_free(&grease);
                free_expanded(expanded_lrows, nor_l);
                free_expanded(expanded_rrows, nor_r);
                free(te_row);
                return cleanup(left_multiplicities, right_multiplicities, dim_irr, dim_end,
                               nor_p, noc_p, len_p, nor_q, noc_q, len_q, NULL, NULL, NULL,
                               NULL, NULL, p, q, h_p, h_q, s, h_o, NULL, rows, lrows, rrows);
              }
              if (0 == mul_in_store(q_rows, end_rows, te_rows, 0 /* is_map1 */, 0 /* is_map2 */,
                                    noc_q[i], len_p[j], nob, dim_end[i], dim_end[j], prime,
                                    &grease, argv[2 * i], "sub-tensor", name)) {
                matrix_free(te_rows);
                matrix_free(end_rows);
                matrix_free(q_rows);
                grease_free(&grease);
                free_expanded(expanded_lrows, nor_l);
                free_expanded(expanded_rrows, nor_r);
                free(te_row);
                return cleanup(left_multiplicities, right_multiplicities, dim_irr, dim_end,
                               nor_p, noc_p, len_p, nor_q, noc_q, len_q, NULL, NULL, NULL,
                               NULL, NULL, p, q, h_p, h_q, s, h_o, NULL, rows, lrows, rrows);
              }
              /* Write into the output */
              for (k = 0; k < dim_end[i]; k++) {
                for (l = 0; l < dim_end[j]; l++) {
                  unsigned int elt = get_element_from_row_with_params(nob, l, mask, elts_per_word, te_rows[k]);
                  if (0 != elt) {
                    put_element_to_row(nob, o_c + l, rows[k], elt);
                  }
                }
              }
              o_c += dim_end[j]; /* Increment output column index */
            }
          }
          m_c += left_multiplicities[j] * dim_irr_j;
          n_c += right_multiplicities[j] * dim_irr_j;
        }
        /* Now write out the rows we've just made */
        if (0 == endian_write_matrix(outp, rows, len, dim_end[i])) {
          matrix_free(te_rows);
          matrix_free(end_rows);
          matrix_free(q_rows);
          grease_free(&grease);
          free_expanded(expanded_lrows, nor_l);
          free_expanded(expanded_rrows, nor_r);
          free(te_row);
          return cleanup(left_multiplicities, right_multiplicities, dim_irr, dim_end,
                         nor_p, noc_p, len_p, nor_q, noc_q, len_q, NULL, NULL, NULL,
                         NULL, NULL, p, q, h_p, h_q, s, h_o, outp, rows, lrows, rrows);
        }
        o_r += dim_end[i]; /* Increment output row index */
      }
    }
    m_r += left_multiplicities[i] * dim_irr_i;
    n_r += right_multiplicities[i] * dim_irr_i;
  }
  matrix_free(te_rows);
  matrix_free(end_rows);
  matrix_free(q_rows);
  grease_free(&grease);
  free_expanded(expanded_lrows, nor_l);
  free_expanded(expanded_rrows, nor_r);
  free(te_row);
  (void)cleanup(left_multiplicities, right_multiplicities, dim_irr, dim_end,
                nor_p, noc_p, len_p, nor_q, noc_q, len_q, NULL, NULL, NULL,
                NULL, NULL, p, q, h_p, h_q, s, h_o, outp, rows, lrows, rrows);
  return 1;
}
