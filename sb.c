/*
 * $Id: sb.c,v 1.1 2001/12/23 23:31:42 jon Exp $
 *
 * Function to spin some vectors under two generators to obtain a standard base
 *
 */

#include "sb.h"
#include "clean.h"
#include "elements.h"
#include "endian.h"
#include "grease.h"
#include "header.h"
#include "matrix.h"
#include "memory.h"
#include "mul.h"
#include "primes.h"
#include "read.h"
#include "utils.h"
#include "write.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

typedef struct gen_struct *gen;

struct gen_struct
{
  FILE *f;
  const char *m;
  unsigned int nor;
  gen next;
};

static void cleanup(FILE *f1, FILE *f2, FILE *f3)
{
  if (NULL != f1)
    fclose(f1);
  if (NULL != f2)
    fclose(f2);
  if (NULL != f3)
    fclose(f3);
}

unsigned int spin(const char *in, const char *out, const char *a,
                  const char *b, const char *name)
{
  FILE *inp = NULL, *outp = NULL, *f_a = NULL, *f_b = NULL;
  const header *h_in, *h_a, *h_b;
  header *h_out;
  unsigned int prime, nob, noc, nor, len, max_rows, d;
  unsigned int **rows1, **rows2;
  int *map, *new_map;
  grease_struct grease;
  prime_ops prime_operations;
  row_ops row_operations;
  struct gen_struct gen_a, gen_b, *gen = &gen_a;
  gen_a.next = &gen_b;
  gen_b.next = &gen_a;
  assert(NULL != in);
  assert(NULL != out);
  assert(NULL != a);
  assert(NULL != b);
  assert(NULL != name);
  if (0 == open_and_read_binary_header(&inp, &h_in, in, name) ||
      0 == open_and_read_binary_header(&f_a, &h_a, a, name) ||
      0 == open_and_read_binary_header(&f_b, &h_b, b, name)) {
    fprintf(stderr, "%s: failed to open or read header from one of %s, %s, %s, terminating\n",
            name, in, a, b);
    cleanup(inp, f_a, f_b);
    exit(1);
  }
  prime = header_get_prime(h_in);
  nob = header_get_nob(h_in);
  noc = header_get_noc(h_in);
  nor = header_get_nor(h_in);
  len = header_get_len(h_in);
  if (noc != header_get_noc(h_a) ||
      noc != header_get_nor(h_a) ||
      noc != header_get_noc(h_b) ||
      noc != header_get_nor(h_b) ||
      prime != header_get_prime(h_a) ||
      prime != header_get_prime(h_b) ||
      nob != header_get_nob(h_a) ||
      nob != header_get_nob(h_b) ||
      1 != nor) {
    fprintf(stderr, "%s: incompatible/bad parameters for %s, %s, %s, terminating\n",
            name, in, a, b);
    cleanup(inp, f_a, f_b);
    exit(1);
  }
  assert(header_get_len(h_a) == len);
  assert(header_get_len(h_b) == len);
  h_out = header_create(prime, nob, header_get_nod(h_in), noc, noc);
  header_free(h_in);
  header_free(h_a);
  header_free(h_b);
  gen_a.f = f_a;
  gen_a.m = a;
  gen_a.nor = 0;
  gen_b.f = f_b;
  gen_b.m = b;
  gen_b.nor = 0;
  primes_init(prime, &prime_operations);
  rows_init(prime, &row_operations);
  grease_init(&row_operations, &grease);
  max_rows = memory_rows(len, 450);
  if (0 == grease_level(prime, &grease, memory_rows(len, 100))) {
    fprintf(stderr, "%s: failed to get grease for %s, %s, %s, terminating\n",
            name, in, a, b);
    cleanup(inp, f_a, f_b);
    exit(2);
  }
  if (max_rows < noc + 1) {
    fprintf(stderr, "%s: failed to get %d + 1 rows for %s, %s, %s, terminating\n",
            name, noc, in, a, b);
    cleanup(inp, f_a, f_b);
    exit(2);
  }
  max_rows = (max_rows > noc * 2) ? noc * 2 : max_rows; /* Can never need more than this */
  rows1 = matrix_malloc(max_rows);
  rows2 = matrix_malloc(max_rows);
  for (d = 0; d < max_rows; d++) {
    rows1[d] = memory_pointer_offset(0, d, len);
    rows2[d] = memory_pointer_offset(450, d, len);
  }
  if (0 == endian_read_matrix(inp, rows1, len, nor)) {
    fprintf(stderr, "%s: failed to read rows from %s, terminating\n",
            name, in);
    cleanup(inp, f_a, f_b);
    exit(1);
  }
  fclose(inp);
  for (d = 0; d < nor; d++) {
    memcpy(rows2[d], rows1[d], len * sizeof(unsigned int));
  }
  map = my_malloc(max_rows * sizeof(int));
  for (d = 0; d < nor; d++) {
    unsigned int i;
    unsigned int elt = first_non_zero(rows1[d], nob, len, &i);
    assert(0 != elt);
    NOT_USED(elt);
    map[d] = i;
  }
  if (0 == grease_allocate(prime, len, &grease, 900)){
    fprintf(stderr, "%s: unable to allocate grease, terminating\n", name);
    cleanup(inp, f_a, f_b);
  }
  while (nor < max_rows && (gen_a.nor < nor || gen_b.nor < nor)) {
    unsigned int rows_to_do = nor - gen->nor, rows_poss = max_rows - nor;
    unsigned int i, j, k, old_nor = nor;
    /* Ensure we don't try to do too many */
    for (k = 0; k < rows_to_do; k += rows_poss) {
      unsigned int stride = (k + rows_poss <= rows_to_do) ? rows_poss : rows_to_do - k;
      if (0 == mul_from_store(rows1 + gen->nor, rows1 + nor, gen->f, noc, len, nob,
                              stride, prime, &grease, gen->m, name)) {
        fprintf(stderr, "%s: failed to multiply using %s, terminating\n", name, gen->m);
        cleanup(inp, f_a, f_b);
        exit(1);
      }
      /* Now copy rows created to rows2 */
      for (i = 0; i < stride; i++) {
        memcpy(rows2[nor + i], rows1[nor + i], len * sizeof(unsigned int));
      }
      gen->nor += stride;
      clean(rows2, nor, rows2 + nor, stride, map, NULL, NULL, 0,
            grease.level, prime, len, nob, 900, 0, 0, name);
      echelise(rows2 + nor, stride, &d, &new_map, NULL, 0,
               grease.level, prime, len, nob, 900, 0, 0, 1, name);
      clean(rows2 + nor, stride, rows2, nor, new_map, NULL, NULL, 0,
            grease.level, prime, len, nob, 900, 0, 0, name);
      j = 0;
      for (i = 0; i < stride; i++) {
        if (new_map[i] >= 0) {
          /* Got a useful row */
          unsigned int *row;
          map[nor + j] = new_map[i];
          /* Swap pointers */
          row = rows1[nor + j];
          rows1[nor + j] = rows1[nor + i];
          rows1[nor + i] = row;
          row = rows2[nor + j];
          rows2[nor + j] = rows2[nor + i];
          rows2[nor + i] = row;
          j++;
        }
      }
      free(new_map);
      assert(j == d);
      nor += d; /* The number of extra rows we made */
    }
    assert(gen->nor == old_nor);
    NOT_USED(old_nor);
    gen = gen->next;
  }
  if (nor >= max_rows) {
    fprintf(stderr, "%s: out of memory at %d rows, terminating\n",
            name, nor);
    exit(2);
  }
  fclose(f_a);
  fclose(f_b);
  if (nor != noc) {
    fprintf(stderr, "%s: fails to spin to full space (%d, %d), terminating\n",
            name, nor, noc);
    exit(1);
  }
  if (0 == open_and_write_binary_header(&outp, h_out, out, name)) {
    exit(1);
  }
  if (0 == endian_write_matrix(outp, rows1, len, nor)) {
    fprintf(stderr, "%s: failed to write output to %s, terminating\n",
            name, out);
    fclose(outp);
    exit(1);
  }
  header_free(h_out);
  matrix_free(rows1);
  matrix_free(rows2);
  grease_free(&grease);
  fclose(outp);
  return nor;
}
