/*
 * $Id: tcv.c,v 1.9 2005/06/22 21:52:54 jon Exp $
 *
 * Function to lift vectors from a tensor condensation representation
 *
 */

#include "tcv.h"
#include <stdio.h>
#include <assert.h>
#include <math.h>
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

static void close_files_and_headers(FILE **q, const header **h_q, u32 i)
{
  u32 j;
  assert(NULL != q);
  assert(NULL != h_q);
  for (j = 0; j < i; j++) {
    assert(NULL != q[j]);
    fclose(q[j]);
    assert(NULL != h_q[j]);
    header_free(h_q[j]);
  }
}

static int cleanup(u32 *left_multiplicities, u32 *right_multiplicities,
                   u32 *nor_q, u32 *noc_q, u32 *len_q, u32 *irr_q,
                   FILE *inp,
                   FILE **q, const header **h_q, u32 i,
                   const header *h_i,
                   const header *h_o, FILE *outp)
{
  if (NULL != left_multiplicities) {
    free(left_multiplicities);
  }
  if (NULL != right_multiplicities) {
    free(right_multiplicities);
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
  if (NULL != irr_q) {
    free(irr_q);
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
  return 0;
}

static void positions(u32 cf, u32 l, u32 r, u32 dim_r,
                      u32 *left_multiplicities, u32 *right_multiplicities,
                      u32 *irr_q, u32 *nor_q, u32 *cpos, u32 *ucpos)
{
  u32 i, start_l = 0, start_r = 0;
  assert(NULL != left_multiplicities);
  assert(NULL != right_multiplicities);
  assert(NULL != irr_q);
  assert(NULL != nor_q);
  assert(NULL != cpos);
  assert(NULL != ucpos);
  assert(0 != dim_r);
  for (i = 0; i < cf; i++) {
    start_l += irr_q[i] * left_multiplicities[i];
    start_r += irr_q[i] * right_multiplicities[i];
  }
  start_l += irr_q[cf] * l;
  start_r += irr_q[cf] * r;
  /*
  printf("start_l = %d, start_r = %d\n", start_l, start_r);
  */
  *ucpos = start_l * dim_r + start_r;
  *cpos = 0;
  for (i = 0; i < cf; i++) {
    *cpos += left_multiplicities[i] * right_multiplicities[i] * nor_q[i];
  }
  *cpos += (l * right_multiplicities[cf] + r) * nor_q[cf];
}

int tco_lift(u32 s, const char *mults_l, const char *mults_r, const char *in,
             const char *out, int argc, const char *const *argv, const char *name)
{
  u32 *left_multiplicities = NULL, *right_multiplicities = NULL;
  FILE *inp = NULL, *outp = NULL, **q;
  const header **h_q, *h_i, *h_o;
  u32 i, j, row, nob, nor_i, noc_i, len_i, noc_o, len_o, prime, max_end, max_irr, max_irr_len, extent_q, extent_i, extent_o, dim_l, dim_r, elts_per_word;
  u32 *nor_q = NULL, *noc_q = NULL, *len_q = NULL, *irr_q = NULL;
  word *row_i, *row_o, *inter_row_i, *inter_row_o;
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
  left_multiplicities = my_malloc(s * sizeof(u32));
  right_multiplicities = my_malloc(s * sizeof(u32));
  inp = fopen(mults_l, "r");
  if (NULL == inp) {
    fprintf(stderr, "%s: failed to open left multiplicities file '%s', terminating\n", name, mults_l);
    return cleanup(left_multiplicities, right_multiplicities,
                   NULL, NULL, NULL, NULL, NULL,
                   NULL, NULL, 0, NULL, NULL, NULL);
  }
  if (0 == read_numbers(inp, s, left_multiplicities)) {
    fprintf(stderr, "%s: failed to read left multiplicities file '%s', terminating\n", name, mults_l);
    return cleanup(left_multiplicities, right_multiplicities,
                   NULL, NULL, NULL, NULL, inp,
                   NULL, NULL, 0, NULL, NULL, NULL);
  }
  fclose(inp);
  inp = fopen(mults_r, "r");
  if (NULL == inp) {
    fprintf(stderr, "%s: failed to open right multiplicities file '%s', terminating\n", name, mults_l);
    return cleanup(left_multiplicities, right_multiplicities,
                   NULL, NULL, NULL, NULL, NULL,
                   NULL, NULL, 0, NULL, NULL, NULL);
  }
  if (0 == read_numbers(inp, s, right_multiplicities)) {
    fprintf(stderr, "%s: failed to read right multiplicities file '%s', terminating\n", name, mults_r);
    return cleanup(left_multiplicities, right_multiplicities,
                   NULL, NULL, NULL, NULL, inp,
                   NULL, NULL, 0, NULL, NULL, NULL);
  }
  fclose(inp);
  if (0 == open_and_read_binary_header(&inp, &h_i, in, name)) {
    return cleanup(left_multiplicities, right_multiplicities,
                   NULL, NULL, NULL, NULL, NULL,
                   NULL, NULL, 0, NULL, NULL, NULL);
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
                     NULL, NULL, NULL, NULL, inp,
                     q, h_q, i, h_i, NULL, NULL);
    }
  }
  nor_q = my_malloc(s * sizeof(u32));
  noc_q = my_malloc(s * sizeof(u32));
  len_q = my_malloc(s * sizeof(u32));
  irr_q = my_malloc(s * sizeof(u32));
  j = 0;
  for (i = 0; i < s; i++) {
    nor_q[i] = header_get_nor(h_q[i]);
    noc_q[i] = header_get_noc(h_q[i]);
    len_q[i] = header_get_len(h_q[i]);
    irr_q[i] = (u32)sqrt(noc_q[i]);
    if (header_get_nob(h_q[i]) != nob || header_get_prime(h_q[i]) != prime ||
        irr_q[i] * irr_q[i] != noc_q[i]) {
      fprintf(stderr, "%s: header incompatibility for '%s', terminating\n", name, argv[i]);
      return cleanup(left_multiplicities, right_multiplicities,
                     nor_q, noc_q, len_q, irr_q, inp,
                     q, h_q, s, h_i, NULL, NULL);
    }
    j += left_multiplicities[i] * right_multiplicities[i] * nor_q[i];
  }
  if (j != noc_i) {
    fprintf(stderr, "%s: header incompatibility for '%s' and the Qi, terminating\n", name, in);
    return cleanup(left_multiplicities, right_multiplicities,
                   nor_q, noc_q, len_q, irr_q, NULL,
                   q, h_q, s, h_i, NULL, NULL);
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
                   nor_q, noc_q, len_q, irr_q, inp,
                   q, h_q, s, h_i, NULL, NULL);
  }
  extent_q = find_extent(s * max_end, max_irr_len);
  extent_i = find_extent(2, len_i);
  dim_l = 0;
  dim_r = 0;
  for (i = 0; i < s; i++) {
    dim_l += left_multiplicities[i] * irr_q[i];
    dim_r += right_multiplicities[i] * irr_q[i];
  }
  noc_o = dim_l * dim_r;
  h_o = header_create(prime, nob, header_get_nod(h_i), noc_o, nor_i);
  len_o = header_get_len(h_o);
  extent_o = find_extent(3, len_o);
  if (extent_q + extent_i + extent_o >= 900) {
    fprintf(stderr, "%s: insufficient memory, terminating\n", name);
    (void)cleanup(left_multiplicities, right_multiplicities,
                  nor_q, noc_q, len_q, irr_q, inp,
                  q, h_q, s, h_i, h_o, outp);
    return 255;
  }
  row_i = memory_pointer(extent_q);
  inter_row_i = memory_pointer_offset(extent_q, 1, len_i);
  row_o = memory_pointer_offset(extent_q + extent_i, 0, len_o);
  inter_row_o = memory_pointer_offset(extent_q + extent_i, 2, len_o);
  if (0 == open_and_write_binary_header(&outp, h_o, out, name)) {
    return cleanup(left_multiplicities, right_multiplicities,
                   nor_q, noc_q, len_q, irr_q, inp,
                   q, h_q, s, h_i, h_o, outp);
  }
  (void)get_mask_and_elts(nob, &elts_per_word);
  for (row = 0; row < nor_i; row++) {
    u32 col_i, col_o;
    if (0 == endian_read_row(inp, row_i, len_i)) {
      fprintf(stderr, "%s: failed to read row %d from %s, terminating\n", name, i, in);
      return cleanup(left_multiplicities, right_multiplicities,
                     nor_q, noc_q, len_q, irr_q, inp,
                     q, h_q, s, h_i, h_o, outp);
    }
    row_init(row_o, len_o);
    for (i = 0; i < s; i++) {
      u32 k, l = left_multiplicities[i], r = right_multiplicities[i], m, n;
      for (j = 0; j < l; j++) {
        for (k = 0; k < r; k++) {
          positions(i, j, k, dim_r, left_multiplicities, right_multiplicities, irr_q, nor_q, &col_i, &col_o);
          assert(col_i < noc_i);
          assert(col_o < noc_o);
          /*
          printf("i = %d, j = %d, k = %d, col_i = %d, col_o = %d\n", i, j, k, col_i, col_o);
          */
          row_init(inter_row_i, len_i);
          for (m = 0; m < nor_q[i]; m++) {
            word elt = get_element_from_row(nob, col_i + m, row_i);
            if (0 != elt) {
              put_element_to_clean_row_with_params(nob, m, elts_per_word, inter_row_i, elt);
            }
          }
          if (0 ==  mul_from_store(&inter_row_i, &inter_row_o,
                                   q[i], 0, nor_q[i], len_q[i],
                                   nob, 1, noc_q[i], prime,
                                   &grease, 0, argv[i], name)) {
            return cleanup(left_multiplicities, right_multiplicities,
                           nor_q, noc_q, len_q, irr_q, inp,
                           q, h_q, s, h_i, h_o, outp);
          }
          n = 0;
          for (m = 0; m < irr_q[i]; m++) {
            /* Copy from inter_row_o to row_o starting at column col_o in row_o, column n in inter_row_o  and incrementing */
            u32 t;
            for (t = 0; t < irr_q[i]; t++) {
              word elt = get_element_from_row(nob, n + t, inter_row_o);
              if (0 != elt) {
                put_element_to_clean_row_with_params(nob, col_o + t, elts_per_word, row_o, elt);
              }
            }
            n += irr_q[i];
            col_o += dim_r;
            /*
            printf("Incrementing n to %d, col_o to %d\n", n, col_o);
            */
          }
        }
      }
    }
    if (0 == endian_write_row(outp, row_o, len_o)) {
      return cleanup(left_multiplicities, right_multiplicities,
                     nor_q, noc_q, len_q, irr_q, inp,
                     q, h_q, s, h_i, h_o, outp);
    }
  }
  (void)cleanup(left_multiplicities, right_multiplicities,
                nor_q, noc_q, len_q, irr_q, inp,
                q, h_q, s, h_i, h_o, outp);
  return 1;
}
