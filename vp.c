/*
 * $Id: vp.c,v 1.7 2002/07/03 12:06:54 jon Exp $
 *
 * Function to permute some vectors under two generators
 *
 */

#include "vp.h"
#include "elements.h"
#include "endian.h"
#include "grease.h"
#include "header.h"
#include "maps.h"
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
#include <errno.h>

typedef struct vec_struct
{
  unsigned int index;
  unsigned int hash;
  unsigned int *row;
} vec;

static unsigned int row_len = 0;

static int compar(const void *e1, const void *e2)
{
  vec **v1 = (vec **)e1;
  vec **v2 = (vec **)e2;
  unsigned int h1 = (*v1)->hash;
  unsigned int h2 = (*v2)->hash;
  if (h1 != h2) {
    return (h1 < h2) ? -1 : 1;
  }
  /* Hashes equal */
  return memcmp((*v1)->row, (*v2)->row, row_len * sizeof(unsigned int));
}

typedef struct gen_struct *gen;

struct gen_struct
{
  FILE *f;
  const char *m;
  unsigned int nor;
  int is_map;
  unsigned int *map;
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

static unsigned int hash_fn(unsigned int *row, unsigned int len)
{
  unsigned int res = 0, i;
  assert(NULL != row);
  for (i = 0; i < len; i++) {
    res ^= row[i];
  }
  return res;
}

unsigned int permute(const char *in, const char *out, const char *a,
                     const char *b, const char *a_out, const char *b_out,
                     int projective, const char *name)
{
  FILE *inp = NULL, *outp = NULL, *f_a = NULL, *f_b = NULL;
  const header *h_in, *h_a, *h_b;
  header *h_out, *h_map;
  unsigned int prime, nob, noc, nor, len, max_rows, d, hash_len;
  unsigned int **rows, *map_a, *map_b;
  unsigned int *hashes;
  grease_struct grease;
  prime_ops prime_operations;
  row_ops row_operations;
  struct gen_struct gen_a, gen_b, *gen = &gen_a;
  vec *records, **record_ptrs;
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
    cleanup(inp, f_a, f_b);
    exit(1);
  }
  prime = header_get_prime(h_in);
  gen_a.is_map = 1 == header_get_prime(h_a);
  gen_b.is_map = 1 == header_get_prime(h_b);
  nob = header_get_nob(h_in);
  noc = header_get_noc(h_in);
  nor = header_get_nor(h_in);
  len = header_get_len(h_in);
  row_len = len;
  if (noc != header_get_noc(h_a) ||
      noc != header_get_nor(h_a) ||
      noc != header_get_noc(h_b) ||
      noc != header_get_nor(h_b) ||
      (prime != header_get_prime(h_a) && 0 == gen_a.is_map) ||
      (prime != header_get_prime(h_b) && 0 == gen_b.is_map) ||
      (nob != header_get_nob(h_a) && 0 == gen_a.is_map) ||
      (nob != header_get_nob(h_b) && 0 == gen_b.is_map)) {
    fprintf(stderr, "%s: incompatible parameters for %s, %s, %s, terminating\n",
            name, in, a, b);
    cleanup(inp, f_a, f_b);
    exit(1);
  }
  hash_len = len / 10;
  if (0 == hash_len) {
    hash_len = 1;
  } else if (10 < hash_len) {
    hash_len = 10;
  }
  assert(gen_a.is_map || header_get_len(h_a) == len);
  assert(gen_b.is_map || header_get_len(h_b) == len);
  h_out = header_create(prime, nob, header_get_nod(h_in), noc, nor);
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
  max_rows = memory_rows(len, 900);
  if (0 == nor) {
    fprintf(stderr, "%s: no rows in input %s, terminating\n", name, in);
    cleanup(inp, f_a, f_b);
    exit(1);
  }
  if (0 == grease_level(prime, &grease, memory_rows(len, 100))) {
    fprintf(stderr, "%s: failed to get grease for %s, %s, %s, terminating\n",
            name, in, a, b);
    cleanup(inp, f_a, f_b);
    exit(2);
  }
  rows = matrix_malloc(max_rows);
  map_a = malloc_map(max_rows);
  map_b = malloc_map(max_rows);
  gen_a.map = map_a;
  gen_b.map = map_b;
  memset(map_a, 0, max_rows * sizeof(unsigned int));
  memset(map_b, 0, max_rows * sizeof(unsigned int));
  hashes = my_malloc(max_rows * sizeof(unsigned int));
  records = my_malloc(max_rows * sizeof(vec));
  record_ptrs = my_malloc(max_rows * sizeof(vec *));
  for (d = 0; d < max_rows; d++) {
    rows[d] = memory_pointer_offset(0, d, len);
    record_ptrs[d] = records + d;
  }
  errno = 0;
  if (0 == endian_read_matrix(inp, rows, len, nor)) {
    if ( 0 != errno) {
      perror(name);
    }
    fprintf(stderr, "%s: failed to read rows from %s, terminating\n", name, in);
    cleanup(inp, f_a, f_b);
    exit(1);
  }
  fclose(inp);
  for (d = 0; d < nor; d++) {
    hashes[d] = hash_fn(rows[d], hash_len);
    records[d].hash = hashes[d];
    records[d].index = d;
    records[d].row = rows[d];
  }
  if (0 == grease_allocate(prime, len, &grease, 900)){
    fprintf(stderr, "%s: unable to allocate grease, terminating\n", name);
    cleanup(inp, f_a, f_b);
  }
  if (projective) {
    for (d = 0; d < nor; d++) {
      unsigned int pos;
      unsigned int elt = first_non_zero(rows[d], nob, len, &pos);
      NOT_USED(pos);
      if (0 != elt && 1 != elt) {
        elt = prime_operations.invert(elt);
        row_operations.scaler_in_place(rows[d], len, elt);
      }
    }
  }
  qsort(record_ptrs, nor, sizeof(vec *), &compar);
  while (nor < max_rows && (gen_a.nor < nor || gen_b.nor < nor)) {
    unsigned int rows_to_do = max_rows - nor;
    unsigned int i, j = 0;
    vec row_vec, **found_row, *row_vec_ptr = &row_vec;
    /* Ensure we don't try to do too many */
    rows_to_do = (rows_to_do + gen->nor > nor) ? (nor - gen->nor) : rows_to_do;
    if (0 == mul_from_store(rows + gen->nor, rows + nor, gen->f, gen->is_map, noc, len, nob,
                            rows_to_do, noc, prime, &grease, gen->m, name)) {
      fprintf(stderr, "%s: failed to multiply using %s, terminating\n", name, gen->m);
      cleanup(NULL, f_a, f_b);
      exit(1);
    }
    for (i = 0; i < rows_to_do; i++) {
      unsigned int hash;
      if (projective) {
        unsigned int pos;
        unsigned int elt = first_non_zero(rows[nor + i], nob, len, &pos);
        NOT_USED(pos);
        if (0 != elt && 1 != elt) {
          elt = prime_operations.invert(elt);
          row_operations.scaler_in_place(rows[nor + i], len, elt);
        }
      }
      hash = hash_fn(rows[nor + i], hash_len);
      row_vec.hash = hash;
      row_vec.index = 0xffffffff;
      row_vec.row = rows[nor + i];
      found_row = bsearch(&row_vec_ptr, record_ptrs, nor, sizeof(vec *), &compar);
      if (NULL == found_row) {
        /* Got a new row */
        /* The image of row gen->nor + i under gen is row nor + j */
        unsigned int *row;
        /* Swap pointers */
        row = rows[nor + j];
        rows[nor + j] = rows[nor + i];
        rows[nor + i] = row;
        hashes[nor + j] = hash;
        gen->map[gen->nor + i] = nor + j;
        row_vec.index = nor + j;
        records[nor + j] = row_vec;
        j++;
      } else {
        /* Got an existing row */
        gen->map[gen->nor + i] = (*found_row)->index;
      }
    }
    gen->nor += rows_to_do;
    nor += j; /* The number of extra rows we made */
    /* Now sort the new rows in */
    qsort(record_ptrs, nor, sizeof(vec *), &compar);
    gen = gen->next;
  }
  if (nor >= max_rows) {
    fprintf(stderr, "%s: out of memory at %d rows, terminating\n",
            name, nor);
    exit(2);
  }
  fclose(f_a);
  fclose(f_b);
  header_set_nor(h_out, nor);
  if (0 == open_and_write_binary_header(&outp, h_out, out, name)) {
    exit(1);
  }
  errno = 0;
  if (0 == endian_write_matrix(outp, rows, len, nor)) {
    if ( 0 != errno) {
      perror(name);
    }
    fprintf(stderr, "%s: failed to write output to %s, terminating\n",
            name, out);
    fclose(outp);
    exit(1);
  }
  fclose(outp);
  header_free(h_out);
  matrix_free(rows);
  /* Now write out the maps */
  h_map = header_create(1, 0, 0, nor, nor);
  if (0 == open_and_write_binary_header(&outp, h_map, a_out, name)) {
    exit(1);
  }
  if (0 == write_map(outp, nor, map_a, name, a_out)) {
    fclose(outp);
    exit(1);
  }
  fclose(outp);
  map_free(map_a);
  if (0 == open_and_write_binary_header(&outp, h_map, b_out, name)) {
    exit(1);
  }
  if (0 == write_map(outp, nor, map_b, name, b_out)) {
    fclose(outp);
    exit(1);
  }
  fclose(outp);
  map_free(map_b);
  header_free(h_map);
  grease_free(&grease);
  free(hashes);
  return nor;
}
