/*
 * $Id: tco.c,v 1.26 2004/11/28 11:36:53 jon Exp $
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
#include "parse.h"
#include "primes.h"
#include "read.h"
#include "rows.h"
#include "utils.h"
#include "write.h"

#define use_minor_tensor 0
#if use_minor_tensor
static void minor_tensor(unsigned int **expanded_rrows, unsigned int beta_i, unsigned int te_j, unsigned int **te_rows,
                         unsigned int te_o_r, unsigned int max_irr_len, unsigned int dim_irr_j, unsigned int *expanded_lrow,
                         unsigned int base_i, row_ops row_operations, unsigned int *te_row, unsigned int nob, unsigned int elts_per_word_nob,
                         unsigned int nobj)
{
  unsigned int te_o_c_word = 0;
  unsigned int te_o_c_offset = 0;
  unsigned int word = 0;
  unsigned int *expanded_rrow = expanded_rrows[beta_i + te_j];
  unsigned int *te_rowsr = te_rows[te_o_r];
  unsigned int *expanded_lrow1 = expanded_lrow + dim_irr_j;
  unsigned int lim = bits_in_unsigned_int - nob;
  assert(bits_in_unsigned_int >= nob);
  row_init(te_rowsr, max_irr_len);
  while (expanded_lrow < expanded_lrow1) {
    /* Columns of M */
    unsigned int elt = *expanded_lrow;
    if (0 != elt) {
      unsigned int *row = expanded_rrow + base_i; /* was rrows */
      unsigned int *row1;
      if (1 != elt) {
        (*row_operations.scaler)(row, te_row, dim_irr_j, elt);
        row = te_row;
      }
      row1 = row + dim_irr_j;
      while (row < row1) {
        elt = *row;
        word |= elt << te_o_c_offset;
        te_o_c_offset += nob;
        if (te_o_c_offset > lim) {
          te_o_c_offset = 0;
          te_rowsr[te_o_c_word] = word;
          te_o_c_word++;
          word = 0;
        }
        row++;
      }
    } else {
      te_o_c_offset += nobj;
      if (te_o_c_offset > lim) {
        te_rowsr[te_o_c_word] = word;
        te_o_c_word += te_o_c_offset / elts_per_word_nob;
        te_o_c_offset %= elts_per_word_nob;
        word = 0;
      }
    }
    expanded_lrow++;
  }
  te_rowsr[te_o_c_word] = word;
}
#endif

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

#define GREASE_MAX 6

int tcondense(unsigned int s, const char *mults_l, const char *mults_r,
              const char *irr, const char *end,
              const char *left, const char *right, const char *out,
              const char *in, unsigned int loop, unsigned int init,
              int argc, const char *const *argv, const char *name)
{
  unsigned int *left_multiplicities = NULL, *right_multiplicities = NULL, *dim_irr = NULL, *dim_end = NULL;
  FILE *inp = NULL, *leftp = NULL, *rightp = NULL, *outp = NULL, **p, **q;
  unsigned int nor, nob, nod, len, prime, nor_l, noc_l, len_l, nor_r, noc_r, len_r,
    max_rows, max_irr, max_irr2, max_irr_len, max_end, i, j, k;
  unsigned int *nor_p = NULL, *nor_q = NULL, *noc_p = NULL, *noc_q = NULL, *len_p = NULL, *len_q = NULL;
  const header *h_l, *h_r, *h_o, **h_p, **h_q;
  unsigned int alpha, beta, gamma, delta, extent_l, extent_r, extent_te, extent_end, extent_q, extent_p, o_r, m_r, n_r;
  unsigned int **rows, **lrows, **rrows, **te_rows, *te_row, **end_rows, **q_rows, **p_rows, *q_row;
  unsigned int mask, elts_per_word, elts_per_word_nob;
  unsigned int **expanded_lrows, **expanded_rrows;
  unsigned int vector[3] = {0, 0, 0};
  row_ops row_operations;
  grease_struct grease;
  unsigned int powers[GREASE_MAX];
  unsigned int lim;
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
    fprintf(stderr, "%s: failed to open right multiplicities file '%s', terminating\n", name, mults_r);
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
  /* Check that loop and init make sense */
  switch (loop) {
  case 0:
    if (init >= s) {
      fprintf(stderr, "%s: cannot restart beyond end of loop 0, terminating\n", name);
      return cleanup(left_multiplicities, right_multiplicities, dim_irr, dim_end,
                     NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
                     NULL, NULL, NULL, NULL, NULL, NULL, 0, NULL, NULL, NULL, NULL, NULL);
    }
    vector[0] = init;
    break;
  case 1:
    if (init >= left_multiplicities[s-1]) {
      fprintf(stderr, "%s: cannot restart beyond end of loop 1, terminating\n", name);
      return cleanup(left_multiplicities, right_multiplicities, dim_irr, dim_end,
                     NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
                     NULL, NULL, NULL, NULL, NULL, NULL, 0, NULL, NULL, NULL, NULL, NULL);
    }
    vector[0] = s-1;
    vector[1] = init;
    break;
  case 2:
    if (init >= right_multiplicities[s-1]) {
      fprintf(stderr, "%s: cannot restart beyond end of loop 2, terminating\n", name);
      return cleanup(left_multiplicities, right_multiplicities, dim_irr, dim_end,
                     NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
                     NULL, NULL, NULL, NULL, NULL, NULL, 0, NULL, NULL, NULL, NULL, NULL);
    }
    vector[0] = s-1;
    vector[1] = left_multiplicities[s-1] - 1;
    vector[2] = init;
    break;
  default:
    fprintf(stderr, "%s: cannot restart further in than loop 2, terminating\n", name);
      return cleanup(left_multiplicities, right_multiplicities, dim_irr, dim_end,
                     NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
                     NULL, NULL, NULL, NULL, NULL, NULL, 0, NULL, NULL, NULL, NULL, NULL);
  }
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
  for (i = 0; i < s; i++) {
    if (0 == dim_irr[i]) {
      fprintf(stderr, "%s: irreducible %d has dimension zero, terminating\n", name, i);
      return cleanup(left_multiplicities, right_multiplicities, dim_irr, dim_end,
                     NULL, NULL, NULL, NULL, NULL, NULL, inp, NULL, NULL,
                     NULL, NULL, NULL, NULL, NULL, NULL, 0, NULL, NULL, NULL, NULL, NULL);
    }
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
  for (i = 0; i < s; i++) {
    if (0 == dim_end[i]) {
      fprintf(stderr, "%s: irreducible %d has endomorphism dimension zero, terminating\n", name, i);
      return cleanup(left_multiplicities, right_multiplicities, dim_irr, dim_end,
                     NULL, NULL, NULL, NULL, NULL, NULL, inp, NULL, NULL,
                     NULL, NULL, NULL, NULL, NULL, NULL, 0, NULL, NULL, NULL, NULL, NULL);
    }
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
  max_irr2 = max_irr * max_irr;
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
  }
  if (0 == open_and_read_binary_header(&rightp, &h_r, right, name)) {
    return cleanup(left_multiplicities, right_multiplicities, dim_irr, dim_end,
                   NULL, NULL, NULL, NULL, NULL, NULL, NULL, leftp, NULL,
                   h_l, NULL, NULL, NULL, NULL, NULL, 0, NULL, NULL, NULL, NULL, NULL);
  }
  nob = header_get_nob(h_l);
  lim = bits_in_unsigned_int - nob;
  mask = get_mask_and_elts(nob, &elts_per_word);
  elts_per_word_nob = elts_per_word * nob;
  nod = header_get_nod(h_l);
  prime = header_get_prime(h_l);
  nor_l = header_get_nor(h_l);
  noc_l = header_get_noc(h_l);
  len_l = header_get_len(h_l);
  nor_r = header_get_nor(h_r);
  noc_r = header_get_noc(h_r);
  len_r = header_get_len(h_r);
  max_irr_len = compute_len(nob, max_irr2);
  if (1 == prime || header_get_prime(h_r) != prime || 0 == is_a_prime_power(prime) ||
      nor_l != noc_l || nor_r != noc_r) {
    fprintf(stderr, "%s: header compatibility failure between '%s' and '%s', terminating\n", name, left, right);
    return cleanup(left_multiplicities, right_multiplicities, dim_irr, dim_end,
                   NULL, NULL, NULL, NULL, NULL, NULL, NULL, leftp, rightp,
                   h_l, h_r, NULL, NULL, NULL, NULL, 0, NULL, NULL, NULL, NULL, NULL);
  }
  /* Check the dimensions add up */
  i = 0;
  j = 0;
  for (k = 0; k < s; k++) {
    i += dim_irr[k] * left_multiplicities[k];
    j += dim_irr[k] * right_multiplicities[k];
  }
  if (i != nor_l || j != nor_r) {
    fprintf(stderr, "%s: dimension compatibility failure between '%s' and '%s' or '%s' and '%s', terminating\n", name, left, mults_l, right, mults_r);
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
  extent_l = find_extent(max_irr, len_l);
  extent_r = find_extent(nor_r, len_r);
  extent_te = find_extent(1 + max_irr2, max_irr_len);
  extent_end = find_extent(max_irr2, max_irr_len);
  extent_q = find_extent(max_end, max_irr_len);
  extent_p = find_extent(max_irr2 * s, max_irr_len);
  if (extent_l + extent_r + extent_te + extent_end + extent_p + extent_q >= 900) {
    fprintf(stderr, "%s: insufficient memory, terminating\n", name);
    (void)cleanup(left_multiplicities, right_multiplicities, dim_irr, dim_end,
                  nor_p, noc_p, len_p, nor_q, noc_q, len_q, NULL, leftp, rightp,
                  h_l, h_r, p, q, h_p, h_q, s, h_o, NULL, NULL, NULL, NULL);
    return 255;
  }
  lrows = matrix_malloc(max_irr); /* Only need max_irr at once */
  rrows = matrix_malloc(nor_r);
  for (i = 0; i < max_irr; i++) {
    lrows[i] = memory_pointer_offset(0, i, len_l);
  }
  for (i = 0; i < nor_r; i++) {
    rrows[i] = memory_pointer_offset(extent_l, i, len_r);
  }
  if (0 == endian_read_matrix(rightp, rrows, len_r, nor_r)) {
    return cleanup(left_multiplicities, right_multiplicities, dim_irr, dim_end,
                   nor_p, noc_p, len_p, nor_q, noc_q, len_q, NULL, leftp, rightp,
                   h_l, h_r, p, q, h_p, h_q, s, NULL, NULL, NULL, lrows, rrows);
  }
  header_free(h_l);
  header_free(h_r);
  fclose(rightp);
  max_rows = memory_rows(len, 900 - extent_l - extent_r - extent_te - extent_end - extent_p - extent_q);
  if (max_rows < max_end) {
    fprintf(stderr, "%s: cannot allocate enough rows for output, terminating\n", name);
    (void)cleanup(left_multiplicities, right_multiplicities, dim_irr, dim_end,
                  nor_p, noc_p, len_p, nor_q, noc_q, len_q, NULL, leftp, NULL,
                  NULL, NULL, p, q, h_p, h_q, s, h_o, NULL, NULL, lrows, rrows);
    return 255;
  }
  if (0 == open_and_write_binary_header(&outp, h_o, out, name)) {
    return cleanup(left_multiplicities, right_multiplicities, dim_irr, dim_end,
                   nor_p, noc_p, len_p, nor_q, noc_q, len_q, NULL, leftp, NULL,
                   NULL, NULL, p, q, h_p, h_q, s, h_o, NULL, NULL, lrows, rrows);
  }
  if (0 != init || 0 != loop) {
    /* Must copy some stuff first */
    FILE *inp;
    const header *h_i;
    unsigned int nrows = 0;
    unsigned int *row = memory_pointer(0);
    /* Compute the number of rows */
    for (i = 0; i < vector[0]; i++) {
      nrows += left_multiplicities[i] * right_multiplicities[i] * dim_end[i];
    }
    i = vector[0];
    for (j = 0; j < vector[1]; j++) {
      nrows += right_multiplicities[i] * dim_end[i];
    }
    for (k = 0; k < vector[2]; k++) {
      nrows += dim_end[i];
    }
    /* Open the input */
    if (0 == open_and_read_binary_header(&inp, &h_i, in, name)) {
      return cleanup(left_multiplicities, right_multiplicities, dim_irr, dim_end,
                     nor_p, noc_p, len_p, nor_q, noc_q, len_q, NULL, leftp, NULL,
                     NULL, NULL, p, q, h_p, h_q, s, h_o, outp, NULL, lrows, rrows);
    }
    /* Check the parameters */
    if (header_get_prime(h_i) != prime ||
        header_get_nor(h_i) != nor ||
        header_get_noc(h_i) != nor) {
      fclose(inp);
      return cleanup(left_multiplicities, right_multiplicities, dim_irr, dim_end,
                     nor_p, noc_p, len_p, nor_q, noc_q, len_q, NULL, leftp, NULL,
                     NULL, NULL, p, q, h_p, h_q, s, h_o, outp, NULL, lrows, rrows);
    }
    /* Delete the header */
    header_free(h_i);
    /* Copy */
    if (0 == endian_copy_matrix(inp, outp, row, len, nrows)) {
      fprintf(stderr, "%s: failed to copy %d rows from %s to %s, terminating\n", name, nrows, in, out);
      fclose(inp);
      return cleanup(left_multiplicities, right_multiplicities, dim_irr, dim_end,
                     nor_p, noc_p, len_p, nor_q, noc_q, len_q, NULL, leftp, NULL,
                     NULL, NULL, p, q, h_p, h_q, s, h_o, outp, NULL, lrows, rrows);
    }
    fclose(inp);
  }
  rows = matrix_malloc(max_end);
  for (i = 0; i < max_end; i++) {
    rows[i] = memory_pointer_offset(extent_l + extent_r + extent_te + extent_end + extent_p + extent_q, i, len);
  }
  te_rows = matrix_malloc(max_irr2);
  end_rows = matrix_malloc(max_irr2);
  q_rows = matrix_malloc(max_end);
  p_rows = matrix_malloc(max_irr2 * s); /* Enough space for all the Pi */
  /* Rows for the partial tensor */
  for (i = 0; i < max_irr2; i++) {
    te_rows[i] = memory_pointer_offset(extent_l + extent_r, i, max_irr_len);
    end_rows[i] = memory_pointer_offset(extent_l + extent_r + extent_te, i, max_irr_len);
  }
  /* A workspace row for creating partial tensors in */
  te_row = /*memory_pointer_offset(extent_l + extent_r, max_irr2, max_irr_len)*/
    my_malloc(noc_r * sizeof(unsigned int));
  /* Rows for the Q matrices */
  for (i = 0; i < max_end; i++) {
    q_rows[i] = memory_pointer_offset(extent_l + extent_r + extent_te + extent_end, i, max_irr_len);
  }
  /* Rows for the P matrices */
  for (i = 0; i < max_irr2 * s; i++) {
    p_rows[i] = memory_pointer_offset(extent_l + extent_r + extent_te + extent_end + extent_q, i, max_irr_len);
  }
  /* Now read the P matrices */
  for (i = 0; i < s; i++) {
    if (0 == endian_read_matrix(p[i], p_rows + i * max_irr2, len_p[i], nor_p[i])) {
      matrix_free(te_rows);
      matrix_free(end_rows);
      matrix_free(q_rows);
      matrix_free(p_rows);
      free(te_row);
      return cleanup(left_multiplicities, right_multiplicities, dim_irr, dim_end,
                     nor_p, noc_p, len_p, nor_q, noc_q, len_q, NULL, leftp, NULL,
                     NULL, NULL, p, q, h_p, h_q, s, h_o, outp, rows, lrows, rrows);
    }
  }
  short_rows_init(prime, &row_operations);
  grease_init(&row_operations, &grease);
  if (0 == grease_level(prime, &grease, memory_rows(max_irr_len, 100))) {
    fprintf(stderr, "%s: failed to determine grease level, terminating\n", name);
    matrix_free(te_rows);
    matrix_free(end_rows);
    matrix_free(q_rows);
    matrix_free(p_rows);
    free(te_row);
    return cleanup(left_multiplicities, right_multiplicities, dim_irr, dim_end,
                   nor_p, noc_p, len_p, nor_q, noc_q, len_q, NULL, leftp, NULL,
                   NULL, NULL, p, q, h_p, h_q, s, h_o, outp, rows, lrows, rrows);
  }
  if (grease.level > GREASE_MAX) {
    grease.level = GREASE_MAX; /* No point in greasing these little multiplications */
  }
  if (verbose) {
    printf("%s: using grease level %d\n", name, grease.level);
    fflush(stdout);
  }
  if (0 == grease_allocate(prime, max_irr_len, &grease, 900)){
    fprintf(stderr, "%s: failed to allocate grease, terminating\n", name);
    matrix_free(te_rows);
    matrix_free(end_rows);
    matrix_free(q_rows);
    matrix_free(p_rows);
    free(te_row);
    return cleanup(left_multiplicities, right_multiplicities, dim_irr, dim_end,
                   nor_p, noc_p, len_p, nor_q, noc_q, len_q, NULL, leftp, NULL,
                   NULL, NULL, p, q, h_p, h_q, s, h_o, outp, rows, lrows, rrows);
  }
  j = 1;
  for (i = 0; i < GREASE_MAX; i++) {
    j *= prime;
    powers[i] = j;
  }
  expanded_lrows = matrix_malloc(max_irr);
  expanded_rrows = matrix_malloc(nor_r);
  for (i = 0; i < max_irr; i++) {
    expanded_lrows[i] = my_malloc(noc_l * sizeof(unsigned int));
  }
  for (i = 0; i < nor_r; i++) {
    expanded_rrows[i] = my_malloc(noc_r * sizeof(unsigned int));
  }
  for (i = 0; i < nor_r; i++) {
    for (j = 0; j < noc_r; j++) {
      expanded_rrows[i][j] = get_element_from_row_with_params(nob, j, mask, elts_per_word, rrows[i]);
    }
  }
  o_r = 0;
  m_r = 0;
  n_r = 0;
  q_row = matrix_malloc(max_irr2); /* Where to record the zero rows */
  for (i = 0; i < s; i++) {
    /* Row loop over distinct irreducibles of H */
    unsigned int dim_irr_i = dim_irr[i];
    unsigned int dim_endi = dim_end[i];
    unsigned int noc_qi = noc_q[i];
    int skipping0 = i < vector[0];
    int equal0 = i == vector[0];
    /* Read ahead q[i], only needed once */
    if (0 == endian_read_matrix(q[i], q_rows, len_q[i], nor_q[i])) {
      matrix_free(te_rows);
      matrix_free(end_rows);
      matrix_free(q_rows);
      matrix_free(q_row);
      matrix_free(p_rows);
      free_expanded(expanded_lrows, max_irr);
      free_expanded(expanded_rrows, nor_r);
      free(te_row);
      return cleanup(left_multiplicities, right_multiplicities, dim_irr, dim_end,
                     nor_p, noc_p, len_p, nor_q, noc_q, len_q, NULL, leftp, NULL,
                     NULL, NULL, p, q, h_p, h_q, s, h_o, outp, rows, lrows, rrows);
    }
    for (j = 0; j < noc_qi; j++) {
      unsigned int acc = 0;
      for (k = 0; k < nor_q[i]; k++) {
        unsigned int elt = get_element_from_row(nob, j, q_rows[k]);
        acc |= elt;
      }
      q_row[j] = acc;
    }
    for (alpha = 0; alpha < left_multiplicities[i]; alpha++) {
      /* Row loop over multiplicity of Si */
      int skipping1 = skipping0 || (equal0 && (alpha < vector[1]));
      int equal1 = (equal0 && (alpha == vector[1]));
      /* First read lrows */
      assert(dim_irr_i <= max_irr);
      if (0 == endian_read_matrix(leftp, lrows, len_l, dim_irr_i)) {
        return cleanup(left_multiplicities, right_multiplicities, dim_irr, dim_end,
                       nor_p, noc_p, len_p, nor_q, noc_q, len_q, NULL, leftp, NULL,
                       NULL, NULL, p, q, h_p, h_q, s, h_o, outp, rows, lrows, rrows);
      }
      /* Now expand these rows */
      for (k = 0; k < dim_irr_i; k++) {
        for (j = 0; j < noc_l; j++) {
          expanded_lrows[k][j] = get_element_from_row_with_params(nob, j, mask, elts_per_word, lrows[k]);
        }
      }
      for (beta = 0; beta < right_multiplicities[i]; beta++) {
        /* Row loop over multiplicity of Si* */
        int skipping2 = skipping1 || (equal1 && (beta < vector[2]));
        if (skipping2) {
#if 0
          printf("Skipping with i = %d, alpha = %d, beta = %d\n",
                 i, alpha, beta);
#endif
        } else {
          unsigned int beta_i = n_r + beta * dim_irr_i;
          unsigned int o_c = 0, m_c = 0, n_c = 0;
          for (j = 0; j < dim_endi; j++) {
            row_init(rows[j], len);
          }
          for (j = 0; j < s; j++) {
            /* Column loop over distinct irreducibles of H */
            unsigned int dim_irr_j = dim_irr[j];
            unsigned int nobj = nob * dim_irr_j;
            unsigned int dim_endj = dim_end[j];
            unsigned int len_pj = len_p[j];
            unsigned int len_qj = len_q[j];
            unsigned int noc_qj = noc_q[j];
            unsigned int left_multiplicitiesj = left_multiplicities[j];
            unsigned int right_multiplicitiesj = right_multiplicities[j];
            unsigned int **p_rowsj = p_rows + j * max_irr2;
            unsigned int p_rowsj_elt = get_element_from_row(nob, 0, *p_rowsj);
            for (gamma = 0; gamma < left_multiplicitiesj; gamma++) {
              /* Column loop over multiplicity of Sj */
              for (delta = 0; delta < right_multiplicitiesj; delta++) {
                /* Column loop over multiplicity of Sj* */
                unsigned int te_i, te_o_r;
                unsigned int base_i = n_c + delta * dim_irr_j;
                unsigned int **arg1, **arg2;
                te_o_r = 0;
                for (te_i = 0; te_i < dim_irr_i; te_i++) {
                  /* Rows of M */
                  unsigned int *expanded_lrow = expanded_lrows[te_i] + m_c + gamma * dim_irr_j;
                  unsigned int te_j;
                  for (te_j = 0; te_j < dim_irr_i; te_j++) {
                    /* Rows of N */
                    if (0 != q_row[te_o_r]) {
#if use_minor_tensor
                      minor_tensor(expanded_rrows, beta_i, te_j, te_rows,
                                   te_o_r, max_irr_len, dim_irr_j, expanded_lrow,
                                   base_i, row_operations, te_row, nob, elts_per_word_nob, nobj);
#else
                      unsigned int te_o_c_word = 0;
                      unsigned int te_o_c_offset = 0;
                      unsigned int word = 0;
                      unsigned int *expanded_rrow = expanded_rrows[beta_i + te_j] + base_i;
                      unsigned int *te_rowsr = te_rows[te_o_r];
                      unsigned int *expanded_lrow1 = expanded_lrow;
                      unsigned int *expanded_lrow2 = expanded_lrow + dim_irr_j;
                      row_init(te_rowsr, max_irr_len);
                      while (expanded_lrow1 < expanded_lrow2) {
                        /* Columns of M */
                        unsigned int elt = *expanded_lrow1;
                        if (0 != elt) {
                          unsigned int *row = expanded_rrow; /* was rrows */
                          unsigned int *row1;
                          if (1 != elt) {
                            (*row_operations.scaler)(row, te_row, dim_irr_j, elt);
                            row = te_row;
                          }
                          row1 = row + dim_irr_j;
                          while (row < row1) {
                            elt = *row;
                            word |= elt << te_o_c_offset;
                            te_o_c_offset += nob;
                            if (te_o_c_offset > lim) {
                              te_o_c_offset = 0;
                              te_rowsr[te_o_c_word] = word;
                              te_o_c_word++;
                              word = 0;
                            }
                            row++;
                          }
                        } else {
                          te_o_c_offset += nobj;
                          if (te_o_c_offset > lim) {
                            te_rowsr[te_o_c_word] = word;
                            te_o_c_word += te_o_c_offset / elts_per_word_nob;
                            te_o_c_offset %= elts_per_word_nob;
                            word = 0;
                          }
                        }
                        expanded_lrow1++;
                      }
                      te_rowsr[te_o_c_word] = word;
#endif
                    }
                    te_o_r++;
                  }
                }
                if (1 == noc_qi && 1 == *q_row) {
                  /* 1 x 1 identity matrix qi, no point in multiply */
                  arg1 = te_rows;
                  arg2 = end_rows;
                } else {
                  if (0 == mul_in_store(q_rows, te_rows, end_rows,
                                        noc_qi, len_qj, nob, dim_endi, prime,
                                        0, &grease)) {
                    matrix_free(te_rows);
                    matrix_free(end_rows);
                    matrix_free(q_rows);
                    matrix_free(q_row);
                    matrix_free(p_rows);
                    grease_free(&grease);
                    free_expanded(expanded_lrows, max_irr);
                    free_expanded(expanded_rrows, nor_r);
                    free(te_row);
                    return cleanup(left_multiplicities, right_multiplicities, dim_irr, dim_end,
                                   nor_p, noc_p, len_p, nor_q, noc_q, len_q, NULL, leftp, NULL,
                                   NULL, NULL, p, q, h_p, h_q, s, h_o, outp, rows, lrows, rrows);
                  }
                  arg1 = end_rows;
                  arg2 = te_rows;
                }
                /* arg1 is the input to the second multiply, and arg2 the result */
                if (1 == noc_qj && 1 == p_rowsj_elt) {
                  /* 1 x 1 identity matrix pj, no point in multiply */
                  arg2 = arg1; /* Result is the same as input */
                } else {
                  if (0 == mul_in_store(arg1, p_rowsj, arg2, noc_qj,
                                        len_pj, nob, dim_endi, prime,
                                        0, &grease)) {
                    matrix_free(te_rows);
                    matrix_free(end_rows);
                    matrix_free(q_rows);
                    matrix_free(q_row);
                    matrix_free(p_rows);
                    grease_free(&grease);
                    free_expanded(expanded_lrows, max_irr);
                    free_expanded(expanded_rrows, nor_r);
                    free(te_row);
                    return cleanup(left_multiplicities, right_multiplicities, dim_irr, dim_end,
                                   nor_p, noc_p, len_p, nor_q, noc_q, len_q, NULL, leftp, NULL,
                                   NULL, NULL, p, q, h_p, h_q, s, h_o, outp, rows, lrows, rrows);
                  }
                }
                /* Write into the output */
                for (k = 0; k < dim_endi; k++) {
                  unsigned int *rowsk =  rows[k];
                  unsigned int *te_rowsk =  arg2[k];
                  unsigned int l;
                  for (l = 0; l < dim_endj; l++) {
                    unsigned int elt = get_element_from_row_with_params(nob, l, mask, elts_per_word, te_rowsk);
                    if (0 != elt) {
                      put_element_to_row(nob, o_c + l, rowsk, elt);
                    }
                  }
                }
                o_c += dim_endj; /* Increment output column index */
              }
            }
            m_c += left_multiplicitiesj * dim_irr_j;
            n_c += right_multiplicitiesj * dim_irr_j;
          }
          /* Now write out the rows we've just made */
          if (0 == endian_write_matrix(outp, rows, len, dim_endi)) {
            matrix_free(te_rows);
            matrix_free(end_rows);
            matrix_free(q_rows);
            matrix_free(q_row);
            matrix_free(p_rows);
            grease_free(&grease);
            free_expanded(expanded_lrows, max_irr);
            free_expanded(expanded_rrows, nor_r);
            free(te_row);
            return cleanup(left_multiplicities, right_multiplicities, dim_irr, dim_end,
                           nor_p, noc_p, len_p, nor_q, noc_q, len_q, NULL, leftp, NULL,
                           NULL, NULL, p, q, h_p, h_q, s, h_o, outp, rows, lrows, rrows);
          }
        }
        o_r += dim_endi; /* Increment output row index */
      }
    }
    m_r += left_multiplicities[i] * dim_irr_i;
    n_r += right_multiplicities[i] * dim_irr_i;
  }
  fclose(leftp);
  matrix_free(te_rows);
  matrix_free(end_rows);
  matrix_free(q_rows);
  matrix_free(q_row);
  matrix_free(p_rows);
  grease_free(&grease);
  free_expanded(expanded_lrows, max_irr);
  free_expanded(expanded_rrows, nor_r);
  free(te_row);
  (void)cleanup(left_multiplicities, right_multiplicities, dim_irr, dim_end,
                nor_p, noc_p, len_p, nor_q, noc_q, len_q, NULL, NULL, NULL,
                NULL, NULL, p, q, h_p, h_q, s, h_o, outp, rows, lrows, rrows);
  return 1;
}
