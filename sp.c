/*
 * $Id: sp.c,v 1.3 2001/11/25 00:17:19 jon Exp $
 *
 * Function to spin some vectors under two generators
 *
 */

#include "sp.h"
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
  FILE *inp, *outp, *f_a, *f_b;
  const header *h_in, *h_a, *h_b;
  header *h_out;
  unsigned int prime, nob, noc, nor, len, max_rows, d;
  unsigned int **rows;
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
  inp = fopen(in, "rb");
  f_a = fopen(a, "rb");
  f_b = fopen(b, "rb");
  if (NULL == inp || NULL == f_a || NULL == f_b) {
    fprintf(stderr, "%s: failed to open one of %s, %s, %s, %s, terminating\n",
            name, in, out, a, b);
    cleanup(inp, f_a, f_b);
    exit(1);
  }
  if (0 == read_binary_header(inp, &h_in, name) ||
      0 == read_binary_header(f_a, &h_a, name) ||
      0 == read_binary_header(f_b, &h_b, name)) {
    fprintf(stderr, "%s: failed to read header from one of %s, %s, %s, terminating\n",
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
      nob != header_get_nob(h_b)) {
    fprintf(stderr, "%s: incompatible parameters for %s, %s, %s, terminating\n",
            name, in, a, b);
    cleanup(inp, f_a, f_b);
    exit(1);
  }
  assert(header_get_len(h_a) == len);
  assert(header_get_len(h_b) == len);
  h_out = header_create(prime, nob, header_get_nod(h_in), noc, nor);
  gen_a.f = f_a;
  gen_a.m = a;
  gen_a.nor = 0;
  gen_b.f = f_b;
  gen_b.m = b;
  gen_b.nor = 0;
  primes_init(prime, &prime_operations);
  rows_init(prime, &row_operations);
  grease_init(&row_operations, &grease);
  max_rows = memory_rows(len, 900);
  if (0 == grease_level(prime, &grease, memory_rows(len, 100))) {
    fprintf(stderr, "%s: failed to get grease for %s, %s, %s, terminating\n",
            name, in, a, b);
    cleanup(inp, f_a, f_b);
    exit(1);
  }
  rows = matrix_malloc(max_rows);
  for (d = 0; d < max_rows; d++) {
    rows[d] = memory_pointer_offset(0, d, len);
  }
  if (0 == endian_read_matrix(inp, rows, len, nor)) {
    fprintf(stderr, "%s: failed to read rows from %s, terminating\n",
            name, in);
    cleanup(inp, f_a, f_b);
    exit(1);
  }
  fclose(inp);
  map = my_malloc(max_rows * sizeof(int));
  for (d = 0; d < nor; d++) {
    unsigned int i;
    unsigned int elt = first_non_zero(rows[d], nob, len, &i);
    assert(0 != elt);
    NOT_USED(elt);
    map[d] = i;
  }
  if (0 == grease_allocate(prime, len, &grease, 900)){
    fprintf(stderr, "%s: unable to allocate grease, terminating\n", name);
    cleanup(inp, f_a, f_b);
  }
  while (nor < max_rows && (gen_a.nor < nor || gen_b.nor < nor)) {
    unsigned int rows_to_do = max_rows - nor;
    unsigned int i, j = 0;
    /* Ensure we don't try to do too many */
    rows_to_do = (rows_to_do + gen->nor > nor) ? (nor - gen->nor) : rows_to_do;
    if (0 == mul_from_store(rows + gen->nor, rows + nor, gen->f, noc, len, nob,
                            rows_to_do, prime, &grease, gen->m, name)) {
      fprintf(stderr, "%s: failed to multiply using %s, terminating\n", name, gen->m);
      cleanup(inp, f_a, f_b);
      exit(1);
    }
    gen->nor += rows_to_do;
    clean(rows, nor, rows + nor, rows_to_do, map, NULL, NULL, 0,
          grease.level, prime, len, nob, 900, 0, 0, name);
    echelise(rows + nor, rows_to_do, &d, &new_map, NULL, 0,
             grease.level, prime, len, nob, 900, 0, 0, 1, name);
    clean(rows + nor, rows_to_do, rows, nor, new_map, NULL, NULL, 0,
          grease.level, prime, len, nob, 900, 0, 0, name);
    for (i = 0; i < rows_to_do; i++) {
      if (new_map[i] >= 0) {
        /* Got a useful row */
        map[nor + j] = new_map[i];
        memcpy(rows[nor + j], rows[nor + i], len * sizeof(unsigned int));
        j++;
      }
    }
    free(new_map);
    assert(j == d);
    nor += d; /* The number of extra rows we made */
    gen = gen->next;
  }
  fclose(f_a);
  fclose(f_b);
  outp = fopen(out, "wb");
  if (NULL == outp) {
    fprintf(stderr, "%s: failed to open output %s, terminating\n",
            name, out);
    exit(1);
  }
  header_set_nor(h_out, nor);
  if (0 == write_binary_header(outp, h_out, out)) {
    fclose(outp);
    exit(1);
  }
  if (0 == endian_write_matrix(outp, rows, len, nor)) {
    fprintf(stderr, "%s: failed to write output to %s, terminating\n",
            name, out);
    fclose(outp);
    exit(1);
  }
  fclose(outp);
  return nor;
}
