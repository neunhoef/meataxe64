/*
 * $Id: tco.c,v 1.30 2005/10/12 18:20:31 jon Exp $
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
static void minor_tensor(word **expanded_rrows, u32 beta_i, u32 te_j, word **te_rows,
                         u32 te_o_r, u32 max_irr_len, u32 dim_irr_j, word *expanded_lrow,
                         u32 base_i, row_ops row_operations, word *te_row, u32 nob, u32 elts_per_word_nob,
                         u32 nobj)
{
  u32 te_o_c_word = 0;
  u32 te_o_c_offset = 0;
  word elts = 0;
  word *expanded_rrow = expanded_rrows[beta_i + te_j];
  word *te_rowsr = te_rows[te_o_r];
  word *expanded_lrow1 = expanded_lrow + dim_irr_j;
  u32 lim = bits_in_word - nob;
  assert(bits_in_word >= nob);
  row_init(te_rowsr, max_irr_len);
  while (expanded_lrow < expanded_lrow1) {
    /* Columns of M */
    word elt = *expanded_lrow;
    if (0 != elt) {
      word *row = expanded_rrow + base_i; /* was rrows */
      word *row1;
      if (1 != elt) {
        (*row_operations.scaler)(row, te_row, dim_irr_j, elt);
        row = te_row;
      }
      row1 = row + dim_irr_j;
      while (row < row1) {
        elt = *row;
        elts |= elt << te_o_c_offset;
        te_o_c_offset += nob;
        if (te_o_c_offset > lim) {
          te_o_c_offset = 0;
          te_rowsr[te_o_c_word] = elts;
          te_o_c_word++;
          elts = 0;
        }
        row++;
      }
    } else {
      te_o_c_offset += nobj;
      if (te_o_c_offset > lim) {
        te_rowsr[te_o_c_word] = elts;
        te_o_c_word += te_o_c_offset / elts_per_word_nob;
        te_o_c_offset %= elts_per_word_nob;
        elts = 0;
      }
    }
    expanded_lrow++;
  }
  te_rowsr[te_o_c_word] = elts;
}
#endif

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
    max_rows, max_irr, max_irr2, max_irr_len, max_end, i, j, k;
  u32 *nor_p = NULL, *nor_q = NULL, *noc_p = NULL, *noc_q = NULL, *len_p = NULL, *len_q = NULL;
  const header *h_l, *h_r, *h_o, **h_p, **h_q;
  u32 alpha, beta, gamma, delta, extent_l, extent_r, extent_te, extent_end, extent_q, extent_p, n_r;
  word **rows, **lrows, **rrows, **te_rows, *te_row, **end_rows, **q_rows, **p_rows, *q_row;
  u32 elts_per_word, elts_per_word_nob;
  word mask;
  word **expanded_lrows, **expanded_rrows;
  u32 v0 = 0, v1 = 0, v2 = 0;
  row_ops row_operations;
  grease_struct grease;
  u32 lim;
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
    v0 = init;
    break;
  case 1:
    if (init >= left_multiplicities[s-1]) {
      fprintf(stderr, "%s: cannot restart beyond end of loop 1, terminating\n", name);
      return cleanup(left_multiplicities, right_multiplicities, dim_irr, dim_end,
                     NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
                     NULL, NULL, NULL, NULL, NULL, NULL, 0, NULL, NULL, NULL, NULL, NULL);
    }
    v0 = s-1;
    v1 = init;
    break;
  case 2:
    if (init >= right_multiplicities[s-1]) {
      fprintf(stderr, "%s: cannot restart beyond end of loop 2, terminating\n", name);
      return cleanup(left_multiplicities, right_multiplicities, dim_irr, dim_end,
                     NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
                     NULL, NULL, NULL, NULL, NULL, NULL, 0, NULL, NULL, NULL, NULL, NULL);
    }
    v0 = s-1;
    v1 = left_multiplicities[s-1] - 1;
    v2 = init;
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
  lim = bits_in_word - nob;
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
    u32 nrows = 0;
    word *row = memory_pointer(0);
    /* Compute the number of rows */
    for (i = 0; i < v0; i++) {
      nrows += left_multiplicities[i] * right_multiplicities[i] * dim_end[i];
    }
    i = v0;
    for (j = 0; j < v1; j++) {
      nrows += right_multiplicities[i] * dim_end[i];
    }
    for (k = 0; k < v2; k++) {
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
      fprintf(stderr, "%s: failed to copy %u rows from %s to %s, terminating\n", name, nrows, in, out);
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
    my_malloc(noc_r * sizeof(word));
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
    printf("%s: using grease level %u\n", name, grease.level);
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
  }
  expanded_lrows = matrix_malloc(max_irr);
  expanded_rrows = matrix_malloc(nor_r);
  for (i = 0; i < max_irr; i++) {
    expanded_lrows[i] = my_malloc(noc_l * sizeof(word));
  }
  for (i = 0; i < nor_r; i++) {
    expanded_rrows[i] = my_malloc(noc_r * sizeof(word));
  }
  for (i = 0; i < nor_r; i++) {
    for (j = 0; j < noc_r; j++) {
      expanded_rrows[i][j] = get_element_from_row_with_params(nob, j, mask, elts_per_word, rrows[i]);
    }
  }
  n_r = 0;
  q_row = matrix_malloc(max_irr2); /* Where to record the zero rows */
  for (i = 0; i < v0; i++) {
    u32 dim_irr_i = dim_irr[i];
    n_r += right_multiplicities[i] * dim_irr[i];
    /* Skip the early rows of the left tensor */
    for (alpha = 0; alpha < left_multiplicities[i] * dim_irr_i; alpha++) {
      endian_skip_row(leftp, len_l);
    }
  }
  for (i = v0; i < s; i++) {
    /* Row loop over distinct irreducibles of H */
    u32 dim_irr_i = dim_irr[i];
    u32 dim_endi = dim_end[i];
    u32 noc_qi = noc_q[i];
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
      word acc = 0;
      for (k = 0; k < nor_q[i]; k++) {
        word elt = get_element_from_row(nob, j, q_rows[k]);
        acc |= elt;
      }
      q_row[j] = acc;
    }
    for (alpha = 0; alpha < v1 * dim_irr_i; alpha++) {
      endian_skip_row(leftp, len_l);
    }
    for (alpha = v1; alpha < left_multiplicities[i]; alpha++) {
      /* Row loop over multiplicity of Si */
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
      for (beta = v2; beta < right_multiplicities[i]; beta++) {
        /* Row loop over multiplicity of Si* */
        {
          u32 beta_i = n_r + beta * dim_irr_i;
          u32 o_c = 0, m_c = 0, n_c = 0;
          for (j = 0; j < dim_endi; j++) {
            row_init(rows[j], len);
          }
          for (j = 0; j < s; j++) {
            /* Column loop over distinct irreducibles of H */
            u32 dim_irr_j = dim_irr[j];
            u32 nobj = nob * dim_irr_j;
            u32 dim_endj = dim_end[j];
            u32 len_pj = len_p[j];
            u32 len_qj = len_q[j];
            u32 noc_qj = noc_q[j];
            u32 left_multiplicitiesj = left_multiplicities[j];
            u32 right_multiplicitiesj = right_multiplicities[j];
            word **p_rowsj = p_rows + j * max_irr2;
            word p_rowsj_elt = get_element_from_row(nob, 0, *p_rowsj);
            for (gamma = 0; gamma < left_multiplicitiesj; gamma++) {
              /* Column loop over multiplicity of Sj */
              for (delta = 0; delta < right_multiplicitiesj; delta++) {
                /* Column loop over multiplicity of Sj* */
                u32 te_i, te_o_r;
                u32 base_i = n_c + delta * dim_irr_j;
                word **arg1, **arg2;
                te_o_r = 0;
                for (te_i = 0; te_i < dim_irr_i; te_i++) {
                  /* Rows of M */
                  word *expanded_lrow = expanded_lrows[te_i] + m_c + gamma * dim_irr_j;
                  u32 te_j;
                  for (te_j = 0; te_j < dim_irr_i; te_j++) {
                    /* Rows of N */
                    if (0 != q_row[te_o_r]) {
#if use_minor_tensor
                      minor_tensor(expanded_rrows, beta_i, te_j, te_rows,
                                   te_o_r, max_irr_len, dim_irr_j, expanded_lrow,
                                   base_i, row_operations, te_row, nob, elts_per_word_nob, nobj);
#else
                      u32 te_o_c_word = 0;
                      u32 te_o_c_offset = 0;
                      word elts = 0;
                      word *expanded_rrow = expanded_rrows[beta_i + te_j] + base_i;
                      word *te_rowsr = te_rows[te_o_r];
                      word *expanded_lrow1 = expanded_lrow;
                      word *expanded_lrow2 = expanded_lrow + dim_irr_j;
                      row_init(te_rowsr, max_irr_len);
                      while (expanded_lrow1 < expanded_lrow2) {
                        /* Columns of M */
                        word elt = *expanded_lrow1;
                        if (0 != elt) {
                          word *row = expanded_rrow; /* was rrows */
                          word *row1;
                          if (1 != elt) {
                            (*row_operations.scaler)(row, te_row, dim_irr_j, elt);
                            row = te_row;
                          }
                          row1 = row + dim_irr_j;
                          while (row < row1) {
                            elt = *row;
                            elts |= elt << te_o_c_offset;
                            te_o_c_offset += nob;
                            if (te_o_c_offset > lim) {
                              te_o_c_offset = 0;
                              te_rowsr[te_o_c_word] = elts;
                              te_o_c_word++;
                              elts = 0;
                            }
                            row++;
                          }
                        } else {
                          te_o_c_offset += nobj;
                          if (te_o_c_offset > lim) {
                            te_rowsr[te_o_c_word] = elts;
                            te_o_c_word += te_o_c_offset / elts_per_word_nob;
                            te_o_c_offset %= elts_per_word_nob;
                            elts = 0;
                          }
                        }
                        expanded_lrow1++;
                      }
                      te_rowsr[te_o_c_word] = elts;
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
                  word *rowsk =  rows[k];
                  word *te_rowsk =  arg2[k];
                  u32 l;
                  for (l = 0; l < dim_endj; l++) {
                    word elt = get_element_from_row_with_params(nob, l, mask, elts_per_word, te_rowsk);
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
      }
      v2 = 0;
    }
    v1 = 0;
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
