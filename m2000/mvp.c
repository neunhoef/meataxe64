/*
 * $Id: mvp.c,v 1.1 2006/07/19 21:26:00 jon Exp $
 *
 * Function to permute some vectors under multiple generators
 *
 */

#include "mvp.h"
#include "elements.h"
#include "endian.h"
#include "gen.h"
#include "grease.h"
#include "header.h"
#include "maps.h"
#include "matrix.h"
#include "memory.h"
#include "mul.h"
#include "primes.h"
#include "parse.h"
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
  u32 index;
  u32 hash;
} vec;

static u32 row_len = 0;

static word **rows;

static unsigned int search_hash_fails = 0;

static unsigned int sort_hash_fails = 0;

static int compar1(const void *e1, const void *e2)
{
  vec **v1 = (vec **)e1;
  vec **v2 = (vec **)e2;
  u32 h1 = (*v1)->hash;
  u32 h2 = (*v2)->hash;
  int res;
  if (h1 != h2) {
    return (h1 < h2) ? -1 : 1;
  }
  /* Hashes equal */
  res = memcmp(rows[(*v1)->index],
               rows[(*v2)->index],
               row_len * sizeof(**rows));
  if (0 != res) {
    search_hash_fails++;
  }
  return res;
}

static int compar2(const void *e1, const void *e2)
{
  vec **v1 = (vec **)e1;
  vec **v2 = (vec **)e2;
  unsigned int h1 = (*v1)->hash;
  unsigned int h2 = (*v2)->hash;
  int res;
  if (h1 != h2) {
    return (h1 < h2) ? -1 : 1;
  }
  /* Hashes equal */
  res = memcmp(rows[(*v1)->index],
               rows[(*v2)->index],
               row_len * sizeof(**rows));
  if (0 != res) {
    sort_hash_fails++;
  }
  return res;
}

static void cleanup(FILE *f1, u32 count, FILE **files)
{
  if (NULL != f1) {
    fclose(f1);
  }
  cleanup_files(files, count);
}

static u32 hash_fn(word *row, u32 len)
{
  u32 i;
  word res = 0;
  assert(NULL != row);
  for (i = 0; i < len; i++) {
    res ^= row[i];
  }
  return res;
}

u32 multi_permute(const char *in, const char *out,
                  u32 argc, const char *const args[],
                  int projective, const char *name)
{
  FILE *inp = NULL, *outp = NULL, **files = NULL;
  const header *h_in;
  header *h_out;
  u32 prime, nob, noc, nor, len, max_rows, d, hash_len;
  u32 grease_memory, grease_start;
  u32 *hashes;
  grease_struct grease;
  prime_ops prime_operations;
  row_ops row_operations;
  struct gen_struct *gens, *gen;
  vec *records, **record_ptrs;
  assert(NULL != in);
  assert(NULL != out);
  assert(NULL != args);
  assert(0 < argc);
  assert(NULL != name);
  if (0 == open_and_read_binary_header(&inp, &h_in, in, name)) {
    cleanup(inp, argc, files);
    exit(1);
  }
  files = my_malloc(argc * sizeof(FILE *));
  gens = my_malloc(argc * sizeof(*gens));
  for (d = 1; d < argc; d++) {
    gens[d - 1].next = gens + d;
    files[d] = NULL;
  }
  gens[argc - 1].next = gens;
  files[0] = NULL;
  /* Start at first generator */
  gen = gens;
  prime = header_get_prime(h_in);
  nob = header_get_nob(h_in);
  noc = header_get_noc(h_in);
  nor = header_get_nor(h_in);
  len = header_get_len(h_in);
  row_len = len;
  for (d = 0; d < argc; d++) {
    const char *gen_name = args[d];
    const header *h;
    if (0 == open_and_read_binary_header(files + d, &h, gen_name, name)) {
      cleanup(inp, argc, files);
      exit(1);
    }
    gens[d].is_map = 1 == header_get_prime(h);
    gens[d].m = gen_name;
    gens[d].f = files[d];
    gens[d].nor = 0;
    if (noc != header_get_noc(h) ||
        noc != header_get_nor(h) ||
        (prime != header_get_prime(h) && 0 == gens[d].is_map) ||
        (nob != header_get_nob(h) && 0 == gens[d].is_map) ||
        (nob != header_get_nob(h) && 0 == gens[d].is_map)) {
      fprintf(stderr, "%s: incompatible parameters for %s, %s, terminating\n",
              name, in, gen_name);
      cleanup(inp, argc, files);
      exit(1);
    }
    assert(gens[d].is_map || header_get_len(h) == len);
    header_free(h);
  }
  hash_len = len / 10;
  if (0 == hash_len) {
    hash_len = 1;
  } else if (10 < hash_len) {
    hash_len = 10;
  }
  h_out = header_create(prime, nob, header_get_nod(h_in), noc, nor);
  header_free(h_in);
  primes_init(prime, &prime_operations);
  rows_init(prime, &row_operations);
  grease_init(&row_operations, &grease);
  if (0 == grease_level(prime, &grease, memory_rows(len, 100))) {
    fprintf(stderr, "%s: failed to get grease for %s, terminating\n",
            name, in);
    cleanup(inp, argc, files);
    exit(2);
  }
  grease_memory = find_extent(grease.size, len);
  assert(grease_memory <= 100);
  grease_start = 1000 - grease_memory;
  max_rows = memory_rows(len, grease_start);
  if (0 == nor) {
    fprintf(stderr, "%s: no rows in input %s, terminating\n", name, in);
    cleanup(inp, argc, files);
    exit(1);
  }
  rows = matrix_malloc(max_rows);
  hashes = my_malloc(max_rows * sizeof(u32));
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
    cleanup(inp, argc, files);
    exit(1);
  }
  fclose(inp);
  for (d = 0; d < nor; d++) {
    hashes[d] = hash_fn(rows[d], hash_len);
    records[d].hash = hashes[d];
    records[d].index = d;
  }
  if (0 == grease_allocate(prime, len, &grease, grease_start)){
    fprintf(stderr, "%s: unable to allocate grease, terminating\n", name);
    cleanup(inp, argc, files);
  }
  if (projective) {
    for (d = 0; d < nor; d++) {
      u32 pos;
      word elt = first_non_zero(rows[d], nob, len, &pos);
      NOT_USED(pos);
      if (0 != elt && 1 != elt) {
        elt = prime_operations.invert(elt);
        row_operations.scaler_in_place(rows[d], len, elt);
      }
    }
  }
  qsort(record_ptrs, nor, sizeof(vec *), &compar2);
  while (nor < max_rows && unfinished(gens, argc, nor)) {
    u32 rows_to_do = max_rows - nor;
    u32 i, j = 0;
    vec row_vec, **found_row, *row_vec_ptr = &row_vec;
    /* Ensure we don't try to do too many */
    rows_to_do = (rows_to_do + gen->nor > nor) ? (nor - gen->nor) : rows_to_do;
    if (0 == mul_from_store(rows + gen->nor, rows + nor, gen->f, gen->is_map, noc, len, nob,
                            rows_to_do, noc, prime, &grease, verbose, gen->m, name)) {
      fprintf(stderr, "%s: failed to multiply using %s, terminating\n", name, gen->m);
      cleanup(NULL, argc, files);
      exit(1);
    }
    for (i = 0; i < rows_to_do; i++) {
      word hash;
      if (projective) {
        u32 pos;
        word elt = first_non_zero(rows[nor + i], nob, len, &pos);
        NOT_USED(pos);
        if (0 != elt && 1 != elt) {
          elt = prime_operations.invert(elt);
          row_operations.scaler_in_place(rows[nor + i], len, elt);
        }
      }
      hash = hash_fn(rows[nor + i], hash_len);
      row_vec.hash = hash;
      row_vec.index = nor + i;
      found_row = bsearch(&row_vec_ptr, record_ptrs, nor, sizeof(vec *), &compar1);
      if (NULL == found_row) {
        /* Got a new row */
        /* The image of row gen->nor + i under gen is row nor + j */
        word *row;
        /* Swap pointers */
        row = rows[nor + j];
        rows[nor + j] = rows[nor + i];
        rows[nor + i] = row;
        hashes[nor + j] = hash;
        row_vec.index = nor + j;
        records[nor + j] = row_vec;
        j++;
      } else {
        /* Got an existing row */
      }
    }
    gen->nor += rows_to_do;
    nor += j; /* The number of extra rows we made */
    /* Now sort the new rows in */
    qsort(record_ptrs, nor, sizeof(vec *), &compar2);
    gen = gen->next;
  }
  if (nor >= max_rows) {
    fprintf(stderr, "%s: out of memory at %u rows, terminating\n",
            name, nor);
    exit(2);
  }
  for (d = 0; d < argc; d++) {
    fclose(files[d]);
  }
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
  grease_free(&grease);
  free(hashes);
  return nor;
}
