/*
 * $Id: ntco.c,v 1.4 2005/12/21 18:12:22 jon Exp $
 *
 * Tensor condense one group element (new algorithm)
 *
 */

#include "newtco.h"
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

static void close_files_and_headers(FILE **p, FILE **q, const header **h_p, const header **h_q, u32 i)
{
  u32 j;
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

static void free_expanded(word **expanded, u32 nor)
{
  u32 i;
  assert(NULL != expanded);
  assert(0 != nor);
  for (i = 0; i < nor; i++) {
    free(expanded[i]);
  }
  matrix_free(expanded);
}

static int cleanup(u32 *left_multiplicities, u32 *right_multiplicities,
                   u32 *dim_irr, u32 *dim_end,
                   u32 *nor_p, u32 *nor_q, u32 *noc_p,
                   u32 *noc_q, u32 *len_p, u32 *len_q,
                   FILE *inp, FILE *leftp, FILE *rightp,
                   const header *h_l, const header *h_r,
                   FILE **p, FILE **q, const header **h_p, const header **h_q, u32 i,
                   const header *h_o, FILE *outp,
                   word **rows, word **lrows, word **rrows)
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
  free((void *)h_p);
  free((void *)h_q);
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

int tcondense(u32 s, const char *mults_l, const char *mults_r,
              const char *irr, const char *end,
              const char *left, const char *right, const char *out,
              const char *in, u32 loop, u32 init,
              int argc, const char *const *argv, const char *name)
{
  u32 *left_multiplicities = NULL, *right_multiplicities = NULL, *dim_irr = NULL, *dim_end = NULL;
  FILE *inp = NULL, *leftp = NULL, *rightp = NULL, *outp = NULL, **p, **q;
  u32 nor, nob, nod, len, prime, nor_l, noc_l, len_l, nor_r, noc_r, len_r,
    max_rows, max_irr, max_irr2, max_irr_len, max_irr2_len, max_end, max_end_len, max_end2_len, max_left, i, j, k, l, m, n;
  u32 *nor_p = NULL, *nor_q = NULL, *noc_p = NULL, *noc_q = NULL, *len_p = NULL, *len_q = NULL;
  const header *h_l, *h_r, *h_o, **h_p, **h_q;
  u32 alpha, beta, gamma, delta, extent_l, extent_r, extent_q, extent_p, extent_sub_q, extent_n, extent_qn, extent_qnp, extent_t, n_r, n_o, m_r, m_l;
  word **rows, **lrows, **rrows, **q_rows, **p_rows, **q_split_rows, **n_rows, **qn_rows, **qnp_rows, *t_row;
  u32 elts_per_word;
  word mask;
  word **expanded_rrows, **expanded_lrows;
  row_ops row_operations;
  row_ops word_row_operations;
  grease_struct grease;
  s64 optr;
  s64 lptr;
  NOT_USED(loop);
  NOT_USED(init);
  NOT_USED(in);
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
    fprintf(stderr, "%s: incorrect number (%d) of arguments Pi, Qi, should be %u, terminating\n", name, argc, 2*s);
    return 0;
  }
  left_multiplicities = my_malloc(s * sizeof(u32));
  right_multiplicities = my_malloc(s * sizeof(u32));
  dim_irr = my_malloc(s * sizeof(u32));
  dim_end = my_malloc(s * sizeof(u32));
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
  max_left = 0;
  for (i = 0; i < s; i++) {
    if (left_multiplicities[i] > max_left) {
      max_left = left_multiplicities[i];
    }
  }
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
      fprintf(stderr, "%s: irreducible %u has dimension zero, terminating\n", name, i);
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
      fprintf(stderr, "%s: irreducible %u has endomorphism dimension zero, terminating\n", name, i);
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
  mask = get_mask_and_elts(nob, &elts_per_word);
  nod = header_get_nod(h_l);
  prime = header_get_prime(h_l);
  nor_l = header_get_nor(h_l);
  noc_l = header_get_noc(h_l);
  len_l = header_get_len(h_l);
  nor_r = header_get_nor(h_r);
  noc_r = header_get_noc(h_r);
  len_r = header_get_len(h_r);
  max_irr2_len = compute_len(nob, max_irr2);
  max_irr_len = compute_len(nob, max_irr);
  max_end_len = compute_len(nob, max_end);
  max_end2_len = compute_len(nob, max_end * max_end);
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
  nor_p = my_malloc(s * sizeof(u32));
  noc_p = my_malloc(s * sizeof(u32));
  len_p = my_malloc(s * sizeof(u32));
  nor_q = my_malloc(s * sizeof(u32));
  noc_q = my_malloc(s * sizeof(u32));
  len_q = my_malloc(s * sizeof(u32));
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
  extent_q = find_extent(max_end, max_irr2_len);
  extent_p = find_extent(max_irr2 * s, max_end_len);
  extent_sub_q = find_extent(max_irr * max_end, max_irr_len); /* Space for the split up Qi */
  extent_n = find_extent(max_irr, max_irr_len); /* Space for the individual N element */
  extent_qn = find_extent(max_end, max_irr_len); /* Space for Qi * N element */
  extent_qnp = find_extent(max_irr2 * max_end, max_end2_len); /* Space for Qi * N * P element */
  extent_t = find_extent(1, max_end2_len); /* Space for T element */
  if (extent_l + extent_r + extent_p + extent_q + extent_sub_q + extent_n + extent_qn + extent_qnp + extent_t >= 900) {
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
  max_rows = memory_rows(len, 900 - extent_l - extent_r - extent_p - extent_q - extent_sub_q - extent_n - extent_qn - extent_qnp - extent_t);
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
  optr = ftello64(outp);
  rows = matrix_malloc(max_end);
  for (i = 0; i < max_end; i++) {
    rows[i] = memory_pointer_offset(extent_l + extent_r + extent_p + extent_q + extent_sub_q + extent_n + extent_qn + extent_qnp + extent_t, i, len);
  }
  row_init(*rows, len);
  for (i = 0; i < nor; i++) {
    if (0 == endian_write_row(outp, *rows, len)) {
      return cleanup(left_multiplicities, right_multiplicities, dim_irr, dim_end,
                     nor_p, noc_p, len_p, nor_q, noc_q, len_q, NULL, leftp, NULL,
                     NULL, NULL, p, q, h_p, h_q, s, h_o, NULL, NULL, lrows, rrows);
    }
  }
  q_rows = matrix_malloc(max_end);
  p_rows = matrix_malloc(max_irr2 * s); /* Enough space for all the Pi */
  /* Rows for the Q matrices */
  for (i = 0; i < max_end; i++) {
    q_rows[i] = memory_pointer_offset(extent_l + extent_r, i, max_irr2_len);
  }
  /* Rows for the P matrices */
  for (i = 0; i < max_irr2 * s; i++) {
    p_rows[i] = memory_pointer_offset(extent_l + extent_r + extent_q, i, max_irr2_len);
  }
  /* Now read the P matrices */
  for (i = 0; i < s; i++) {
    if (0 == endian_read_matrix(p[i], p_rows + i * max_irr2, len_p[i], nor_p[i])) {
      matrix_free(q_rows);
      matrix_free(p_rows);
      return cleanup(left_multiplicities, right_multiplicities, dim_irr, dim_end,
                     nor_p, noc_p, len_p, nor_q, noc_q, len_q, NULL, leftp, NULL,
                     NULL, NULL, p, q, h_p, h_q, s, h_o, outp, rows, lrows, rrows);
    }
  }
  short_rows_init(prime, &row_operations);
  if (max_end2_len <= 1) {
    word_rows_init(prime, &word_row_operations);
  } else {
    short_rows_init(prime, &word_row_operations);
  }
  grease_init(&row_operations, &grease);
  if (0 == grease_level(prime, &grease, memory_rows(max_irr2_len, 100))) { /*** Think about using max_irr_len */
    fprintf(stderr, "%s: failed to determine grease level, terminating\n", name);
    matrix_free(q_rows);
    matrix_free(p_rows);
    return cleanup(left_multiplicities, right_multiplicities, dim_irr, dim_end,
                   nor_p, noc_p, len_p, nor_q, noc_q, len_q, NULL, leftp, NULL,
                   NULL, NULL, p, q, h_p, h_q, s, h_o, outp, rows, lrows, rrows);
  }
  if (grease.level > GREASE_MAX) {
    grease.level = GREASE_MAX; /* No point in greasing these little multiplications */
  }
  if (verbose) {
    printf("%s: using grease level %u\n", name, grease.level);
    fflush(stdout);
  }
  if (0 == grease_allocate(prime, max_irr2_len, &grease, 900)){ /*** Think about using max_irr_len */
    fprintf(stderr, "%s: failed to allocate grease, terminating\n", name);
    matrix_free(q_rows);
    matrix_free(p_rows);
    return cleanup(left_multiplicities, right_multiplicities, dim_irr, dim_end,
                   nor_p, noc_p, len_p, nor_q, noc_q, len_q, NULL, leftp, NULL,
                   NULL, NULL, p, q, h_p, h_q, s, h_o, outp, rows, lrows, rrows);
  }
  j = 1;
  for (i = 0; i < GREASE_MAX; i++) {
    j *= prime;
  }
  expanded_rrows = matrix_malloc(nor_r);
  for (i = 0; i < nor_r; i++) {
    expanded_rrows[i] = my_malloc(noc_r * sizeof(word));
  }
  for (i = 0; i < nor_r; i++) {
    for (j = 0; j < noc_r; j++) {
      expanded_rrows[i][j] = get_element_from_row_with_params(nob, j, mask, elts_per_word, rrows[i]);
    }
  }
  expanded_lrows = matrix_malloc(max_irr);
  for (i = 0; i < max_irr; i++) {
    expanded_lrows[i] = my_malloc(max_left * max_irr * sizeof(word));
  }
  /* Matrix for splitting Qi */
  q_split_rows = matrix_malloc(max_end * max_irr);
  for (i = 0; i < max_irr * max_end; i++) {
    /* Now set up the pointers */
    q_split_rows[i] = memory_pointer_offset(extent_l + extent_r + extent_q + extent_p, i, max_irr_len);
  }
  /* Matrix for the N from the right hand parameter */
  n_rows = matrix_malloc(max_irr);
  for (i = 0; i < max_irr; i++) {
    /* Now set up the pointers */
    n_rows[i] = memory_pointer_offset(extent_l + extent_r + extent_p + extent_q + extent_sub_q, i, max_irr_len);
  }
  qn_rows = matrix_malloc(max_end);
  for (i = 0; i < max_end; i++) {
    /* Now set up the pointers */
    qn_rows[i] = memory_pointer_offset(extent_l + extent_r + extent_p + extent_q + extent_sub_q + extent_n, i, max_irr_len);
  }
  qnp_rows = matrix_malloc(max_irr2 * max_end);
  for (i = 0; i < max_irr2 * max_end; i++) {
    /* Now set up the pointers */
    qnp_rows[i] = memory_pointer_offset(extent_l + extent_r + extent_p + extent_q + extent_sub_q + extent_n + extent_qn, i, max_end2_len);
  }
  /* Now set up the pointers */
  t_row = memory_pointer_offset(extent_l + extent_r + extent_p + extent_q + extent_sub_q + extent_n + extent_qn + extent_qnp, 0, max_end2_len);
  n_r = 0;
  n_o = 0;
  lptr = ftello64(leftp);
  for (i = 0; i < s; i++) {
    /* Row loop over distinct irreducibles of H */
    u32 dim_irr_i = dim_irr[i];
    u32 dim_end_i = dim_end[i];
    u32 m_o = 0;
    /* Read ahead q[i], only needed once */
    if (0 == endian_read_matrix(q[i], q_rows, len_q[i], nor_q[i])) {
      matrix_free(q_rows);
      matrix_free(p_rows);
      matrix_free(q_split_rows);
      matrix_free(n_rows);
      matrix_free(qn_rows);
      matrix_free(qnp_rows);
      grease_free(&grease);
      free_expanded(expanded_rrows, nor_r);
      free_expanded(expanded_lrows, max_irr);
      return cleanup(left_multiplicities, right_multiplicities, dim_irr, dim_end,
                     nor_p, noc_p, len_p, nor_q, noc_q, len_q, NULL, leftp, NULL,
                     NULL, NULL, p, q, h_p, h_q, s, h_o, outp, rows, lrows, rrows);
    }
    /* Split Qi into subrows */
    for (j = 0; j < dim_irr_i; j++) {
      u32 m = j * dim_end_i;
      u32 n = j * dim_irr_i;
      for (k = 0; k < dim_end_i; k++) {
        /* Initialise the result */
        row_init(q_split_rows[k + m], max_irr_len);
        /* Now copy in */
        for (l = 0; l < dim_irr_i; l++) {
          /* extract from q_rows[k] at offset n + l */
          word elt = get_element_from_row_with_params(nob, n + l, mask, elts_per_word, q_rows[k]);
          /* Write at q_split_rows + k + m at offset l */
          put_element_to_clean_row_with_params(nob, l, elts_per_word, q_split_rows[k + m], elt);
        }
      }
    }
    m_r = 0;
    m_l = 0;
    for (j = 0; j < s; j++) {
      word **pj_rows = p_rows + j * max_irr2; /* Pointer to this Pj matrix */
      u32 dim_irr_j = dim_irr[j];
      u32 dim_end_j = dim_end[j];
      u32 len_qj = compute_len(nob, dim_irr_j);
      u32 len_pj = len_p[j];
      /* Component l is at pj_rows + l * dim_irr_j */
      for (beta = 0; beta < right_multiplicities[i]; beta++) {
        for (delta = 0; delta < right_multiplicities[j]; delta++) {
          /* Acquire Ni,beta,j,delta */
          for (k = 0; k < dim_irr_i; k++) {
            row_init(n_rows[k], max_irr_len);
            for(l = 0; l < dim_irr_j; l++) {
              put_element_to_clean_row_with_params(nob, l, elts_per_word, n_rows[k], expanded_rrows[n_r + beta * dim_irr_i + k][m_r + delta * dim_irr_j + l]);
            }
          }
          /* Space for results is max_irr2 * max_end of length len(max_end) */
          /* Compute Rkl = Qik * Ni,beta,j,delta * Pjl */
          for (k = 0; k < dim_irr_i; k++) {
            /* Now multiply on left by Qk */
            if (1 == dim_irr_i && 1 == **q_split_rows) {
              /* 1 x 1 identity matrix */
              /* Copy N into qn_rows */
              for (l = 0; l < len_qj; l++) {
                qn_rows[0][l] = n_rows[0][l];
              }
            } else {
              /* Do the multiply */
              if (0 == mul_in_store(q_split_rows + k * dim_end_i, n_rows, qn_rows,
                                    dim_irr_i, len_qj, nob, dim_end_i, prime, 0, &grease)) {
                matrix_free(q_rows);
                matrix_free(p_rows);
                matrix_free(q_split_rows);
                matrix_free(n_rows);
                matrix_free(qn_rows);
                matrix_free(qnp_rows);
                grease_free(&grease);
                free_expanded(expanded_rrows, nor_r);
                free_expanded(expanded_lrows, max_irr);
                return cleanup(left_multiplicities, right_multiplicities, dim_irr, dim_end,
                               nor_p, noc_p, len_p, nor_q, noc_q, len_q, NULL, leftp, NULL,
                               NULL, NULL, p, q, h_p, h_q, s, h_o, outp, rows, lrows, rrows);
              }
            }
            for (l = 0; l < dim_irr_j; l++) {
              /* Now multiply on right by Pl */
              if (0 == mul_in_store(qn_rows, pj_rows + l * dim_irr_j, qnp_rows + dim_end_i * (k * dim_irr_j + l),
                                    dim_irr_j, len_pj, nob, dim_end_i, prime, 0, &grease)) {
                matrix_free(q_rows);
                matrix_free(p_rows);
                matrix_free(q_split_rows);
                matrix_free(n_rows);
                matrix_free(qn_rows);
                matrix_free(qnp_rows);
                grease_free(&grease);
                free_expanded(expanded_rrows, nor_r);
                free_expanded(expanded_lrows, max_irr);
                return cleanup(left_multiplicities, right_multiplicities, dim_irr, dim_end,
                               nor_p, noc_p, len_p, nor_q, noc_q, len_q, NULL, leftp, NULL,
                               NULL, NULL, p, q, h_p, h_q, s, h_o, outp, rows, lrows, rrows);
              }
              /* Now convert this to linearised form */
              for (m = 1; m < dim_end_i; m++) {
                /* Over the rows */
                for (n = 0; n < dim_end_j; n++) {
                  word elt = get_element_from_row_with_params(nob, n, mask, elts_per_word, qnp_rows[dim_end_i * (k * dim_irr_j + l) + m]);
                  put_element_to_row_with_params(nob, n + m * dim_end_j, mask, elts_per_word, qnp_rows[dim_end_i * (k * dim_irr_j + l)], elt);
                }
              }
            }
          }
          fseeko64(leftp, lptr, SEEK_SET); /* Back to start of rows for i */
          for (alpha = 0; alpha < left_multiplicities[i]; alpha++) {
            /* Seek to correct place in output (accounting for alpha and beta) */
            fseeko64(outp, optr + (s64)((s64)beta + (s64)alpha * (s64)right_multiplicities[i]) *
                     (s64)dim_end_i * (s64)len * (s64)sizeof(word), SEEK_SET);
            /* Get relevant rows of output for update */
            if (0 == endian_read_matrix(outp, rows, len, dim_end_i)) {
              matrix_free(q_rows);
              matrix_free(p_rows);
              matrix_free(q_split_rows);
              matrix_free(n_rows);
              matrix_free(qn_rows);
              matrix_free(qnp_rows);
              grease_free(&grease);
              free_expanded(expanded_rrows, nor_r);
              free_expanded(expanded_lrows, max_irr);
              return cleanup(left_multiplicities, right_multiplicities, dim_irr, dim_end,
                             nor_p, noc_p, len_p, nor_q, noc_q, len_q, NULL, leftp, NULL,
                             NULL, NULL, p, q, h_p, h_q, s, h_o, outp, rows, lrows, rrows);
            }
            /* Get Mi,alpha */
            if (0 == endian_read_matrix(leftp, lrows, len_l, dim_irr_i)) {
              matrix_free(q_rows);
              matrix_free(p_rows);
              matrix_free(q_split_rows);
              matrix_free(n_rows);
              matrix_free(qn_rows);
              matrix_free(qnp_rows);
              grease_free(&grease);
              free_expanded(expanded_rrows, nor_r);
              free_expanded(expanded_lrows, max_irr);
              return cleanup(left_multiplicities, right_multiplicities, dim_irr, dim_end,
                             nor_p, noc_p, len_p, nor_q, noc_q, len_q, NULL, leftp, NULL,
                             NULL, NULL, p, q, h_p, h_q, s, h_o, outp, rows, lrows, rrows);
            }
            /* Now expand the bits we want into expanded_lrows */
            for (k = 0; k < dim_irr_i; k++) {
              get_elements_from_row_with_params_into_row(nob, m_l, mask,
                                                         elts_per_word, lrows[k],
                                                         left_multiplicities[j] * dim_irr_j, expanded_lrows[k]);

            }
            for (gamma = 0; gamma < left_multiplicities[j]; gamma++) {
              /* Initialise the result */
              u32 len_pjl = compute_len(nob, dim_end_i * dim_end_j);
              row_init(t_row, len_pjl);
              /* Form T = Sigma(k,l) Lambdakl*Rkl */
              for (k = 0; k < dim_irr_i; k++) {
                for (l = 0; l < dim_irr_j; l++) {
                  /* Get Lambda = Mi,alpha,j,gamma[k,l] */
                  word elt = expanded_lrows[k][gamma * dim_irr_j + l];
                  /* Add elt * qnp[k,l] to t_row */
                  if (0 != elt) {
                    if (1 == elt) {
                      word_row_operations.incer(qnp_rows[dim_end_i * (k * dim_irr_j + l)], t_row,
                                                len_pjl);
                    } else {
                      word_row_operations.scaled_incer(qnp_rows[dim_end_i * (k * dim_irr_j + l)], t_row,
                                                       len_pjl, elt);
                    }
                  }
                }
              }
              /* Write T into the correct place in outp */
              m = m_o + dim_end_j * (gamma * right_multiplicities[j] + delta);
              for (k = 0; k < dim_end_i; k++) {
                for (l = 0; l < dim_end_j; l++) {
                  /* Extract from t_row at k * dim_end_j + l */
                  word elt = get_element_from_row_with_params(nob, k * dim_end_j + l, mask, elts_per_word, t_row);
                  /* Insert into rows[k] at m + l */
                  put_element_to_clean_row_with_params(nob, m + l, elts_per_word, rows[k], elt);
                }
              }
            } /* gamma */
            /* Write out updated rows */
            fseeko64(outp, optr + (s64)((s64)beta + (s64)alpha * (s64)right_multiplicities[i]) *
                     (s64)dim_end_i * (s64)len * (s64)sizeof(word), SEEK_SET);
            if (0 == endian_write_matrix(outp, rows, len, dim_end_i)) {
              matrix_free(q_rows);
              matrix_free(p_rows);
              matrix_free(q_split_rows);
              matrix_free(n_rows);
              matrix_free(qn_rows);
              matrix_free(qnp_rows);
              grease_free(&grease);
              free_expanded(expanded_rrows, nor_r);
              free_expanded(expanded_lrows, max_irr);
              return cleanup(left_multiplicities, right_multiplicities, dim_irr, dim_end,
                             nor_p, noc_p, len_p, nor_q, noc_q, len_q, NULL, leftp, NULL,
                             NULL, NULL, p, q, h_p, h_q, s, h_o, NULL, NULL, lrows, rrows);
            }
          } /* alpha */
        } /* delta */
      } /* beta */
      m_r += dim_irr_j * right_multiplicities[j];
      m_l += dim_irr_j * left_multiplicities[j];
      m_o += dim_end_j * right_multiplicities[j] * left_multiplicities[j];
    } /* j */
    n_r += dim_irr_i * right_multiplicities[i];
    n_o += dim_end_i * right_multiplicities[i] * left_multiplicities[i];
    lptr = lptr + (s64)dim_irr_i * (s64)left_multiplicities[i] * (s64)len_l * (s64)sizeof(word);
    optr = optr + (s64)dim_end_i * (s64)left_multiplicities[i] * (s64)right_multiplicities[i] *
      (s64)len * (s64)sizeof(word);
  } /* i */
  fclose(leftp);
  matrix_free(q_rows);
  matrix_free(p_rows);
  matrix_free(q_split_rows);
  matrix_free(n_rows);
  matrix_free(qn_rows);
  matrix_free(qnp_rows);
  grease_free(&grease);
  free_expanded(expanded_rrows, nor_r);
  free_expanded(expanded_lrows, max_irr);
  (void)cleanup(left_multiplicities, right_multiplicities, dim_irr, dim_end,
                nor_p, noc_p, len_p, nor_q, noc_q, len_q, NULL, NULL, NULL,
                NULL, NULL, p, q, h_p, h_q, s, h_o, outp, rows, lrows, rrows);
  return 1;
}
