/*
 * $Id: vpf.c,v 1.5 2005/07/24 09:32:46 jon Exp $
 *
 * Function to permute some vectors under two generators,
 * using intermediate file
 */

#include "vpf.h"
#include "elements.h"
#include "endian.h"
#include "files.h"
#include "grease.h"
#include "header.h"
#include "maps.h"
#include "matrix.h"
#include "memory.h"
#include "mul.h"
#include "primes.h"
#include "parse.h"
#include "read.h"
#include "system.h"
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

static int hash_compar(const void *e1, const void *e2)
{
  vec **v1 = (vec **)e1;
  vec **v2 = (vec **)e2;
  u32 h1 = (*v1)->hash;
  u32 h2 = (*v2)->hash;
  return (h1 < h2) ? -1 : (h1 == h2) ? 0 : 1;
}

typedef struct gen_struct *gen;

struct gen_struct
{
  FILE *f;
  const char *m;
  u32 nor;
  int is_map;
  word *map;
  s64 base_ptr;	/* Pointer to row nor + 1 in output file */
  gen next;
};

static void cleanup(FILE *f1, FILE *f2, FILE *f3, FILE *tmp)
{
  if (NULL != f1)
    fclose(f1);
  if (NULL != f2)
    fclose(f2);
  if (NULL != f3)
    fclose(f3);
  if (NULL != tmp)
    fclose(tmp);
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

u32 permute_file(const char *tmp_dir, const char *in,
                 const char *out, const char *a,
                 const char *b, const char *a_out, const char *b_out,
                 int projective, const char *name)
{
  FILE *inp = NULL, *outp = NULL, *f_a = NULL, *f_b = NULL, *vectors = NULL;
  const header *h_in, *h_a, *h_b;
  header *h_out, *h_map;
  u32 prime, nob, noc, nor, len, max_rows, max_rows2, d, hash_len;
  u32 grease_memory, grease_start;
  word *map_a, *map_b;
  u32 *hashes;
  s64 end_ptr = 0;
  grease_struct grease;
  prime_ops prime_operations;
  row_ops row_operations;
  struct gen_struct gen_a, gen_b, *gen = &gen_a;
  vec *records, **record_ptrs;
  char *name_tmp = NULL;
  const char *tmp = tmp_name();
  gen_a.next = &gen_b;
  gen_b.next = &gen_a;
  assert(NULL != in);
  assert(NULL != out);
  assert(NULL != a);
  assert(NULL != b);
  assert(NULL != name);
  if (0 == maximum_rows) {
    fprintf(stderr, "%s: maximum_rows unset, terminating\n", name);
    exit(1);
  }
  if (0 == open_and_read_binary_header(&inp, &h_in, in, name) ||
      0 == open_and_read_binary_header(&f_a, &h_a, a, name) ||
      0 == open_and_read_binary_header(&f_b, &h_b, b, name)) {
    cleanup(inp, f_a, f_b, NULL);
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
    cleanup(inp, f_a, f_b, NULL);
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
  if (0 == grease_level(prime, &grease, memory_rows(len, 100))) {
    fprintf(stderr, "%s: failed to get grease for %s, %s, %s, terminating\n",
            name, in, a, b);
    cleanup(inp, f_a, f_b, NULL);
    exit(2);
  }
  grease_memory = find_extent(grease.size, len);
  assert(grease_memory <= 100);
  grease_start = 1000 - grease_memory;
  max_rows = memory_rows(len, grease_start);
  if (0 == nor) {
    fprintf(stderr, "%s: no rows in input %s, terminating\n", name, in);
    cleanup(inp, f_a, f_b, NULL);
    exit(1);
  }
  max_rows2 = max_rows / 2;
  if (0 == max_rows2) {
    fprintf(stderr, "%s: cannot allocate 2 rows, terminating\n", name);
    cleanup(inp, f_a, f_b, NULL);
    exit(1);
  }
  rows = matrix_malloc(max_rows);
  map_a = malloc_map(maximum_rows);
  map_b = malloc_map(maximum_rows);
  gen_a.map = map_a;
  gen_b.map = map_b;
  gen_a.base_ptr = 0;
  gen_b.base_ptr = 0;
  memset(map_a, 0, maximum_rows * sizeof(*map_a));
  memset(map_b, 0, maximum_rows * sizeof(*map_b));
  hashes = my_malloc(maximum_rows * sizeof(*hashes));
  records = my_malloc(maximum_rows * sizeof(vec));
  record_ptrs = my_malloc(maximum_rows * sizeof(vec *));
  for (d = 0; d < max_rows; d++) {
    rows[d] = memory_pointer_offset(0, d, len);
  }
  for (d = 0; d < maximum_rows; d++) {
    record_ptrs[d] = records + d;
  }
  errno = 0;
  if (0 == endian_read_matrix(inp, rows, len, nor)) {
    if ( 0 != errno) {
      perror(name);
    }
    fprintf(stderr, "%s: failed to read rows from %s, terminating\n", name, in);
    cleanup(inp, f_a, f_b, NULL);
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
    cleanup(inp, f_a, f_b, NULL);
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
  /* Consider detecting duplicates */
  /* Handle the temporary file */
  d = strlen(tmp) + strlen(tmp_dir);
  name_tmp = my_malloc(d + 4);
  sprintf(name_tmp, "%s/%s.1", tmp_dir, tmp);
  /* Create the temporary file */
  errno = 0;
  vectors = fopen64(name_tmp, "w+b");
  if (NULL == vectors) {
    if ( 0 != errno) {
      perror(name);
    }
    fprintf(stderr, "%s: cannot open %s, terminating\n", name, name_tmp);
    cleanup(inp, f_a, f_b, vectors);
    (void)remove(name_tmp);
    exit(1);
  }
  errno = 0;
  if (0 == endian_write_matrix(vectors, rows, len, nor)) {
    if ( 0 != errno) {
      perror(name);
    }
    fprintf(stderr, "%s: failed to write %u rows to %s at offset %lld, terminating\n", name, nor, name_tmp, gen->base_ptr);
    cleanup(NULL, f_a, f_b, vectors);
    (void)remove(name_tmp);
    exit(1);
  }
  end_ptr = ftello64(vectors);
  assert(end_ptr == ((s64)sizeof(**rows)) * ((s64)len) * ((s64)nor));
  /* Sort initial rows if necessary */
  if (nor > 1) {
    qsort(record_ptrs, nor, sizeof(vec *), &hash_compar);
  }
  while (nor < maximum_rows && (gen_a.nor < nor || gen_b.nor < nor)) {
    u32 rows_to_do = maximum_rows - nor;
    u32 temp_space;
    u32 i, j = 0;
    vec row_vec, **found_row, *row_vec_ptr = &row_vec;
    /* Ensure we don't try to do too many */
    rows_to_do = (rows_to_do + gen->nor > nor) ? (nor - gen->nor) : rows_to_do;
    rows_to_do = (rows_to_do > max_rows2) ? max_rows2 : rows_to_do;
    fseeko64(vectors, gen->base_ptr, SEEK_SET);
    errno = 0;
    if (0 == endian_read_matrix(vectors, rows, len, rows_to_do)) {
      if ( 0 != errno) {
        perror(name);
      }
      fprintf(stderr, "%s: failed to read %u rows from %s at offset %lld, terminating\n", name, rows_to_do, name_tmp, gen->base_ptr);
      cleanup(NULL, f_a, f_b, vectors);
      (void)remove(name_tmp);
      exit(1);
    }
    gen->base_ptr = ftello64(vectors); /* Reset the pointer into the rows for this generator */
    assert(gen->base_ptr == sizeof(**rows) * len * (gen->nor + rows_to_do));
    /* Multiply placing results at max_rows - rows_to_do */
    if (0 == mul_from_store(rows, rows + max_rows - rows_to_do, gen->f,
                            gen->is_map, noc, len, nob, rows_to_do, noc,
                            prime, &grease, verbose, gen->m, name)) {
      fprintf(stderr, "%s: failed to multiply using %s, terminating\n", name, gen->m);
      cleanup(NULL, f_a, f_b, vectors);
      (void)remove(name_tmp);
      exit(1);
    }
    /* First, deal with projectivity */
    for (i = 0; i < rows_to_do; i++) {
      if (projective) {
        u32 pos;
        word elt =
          first_non_zero(rows[max_rows + i - rows_to_do], nob, len, &pos);
        NOT_USED(pos);
        if (0 != elt && 1 != elt) {
          elt = prime_operations.invert(elt);
          row_operations.scaler_in_place(rows[max_rows + i - rows_to_do],
                                         len, elt);
        }
      }
    }
    temp_space = max_rows - rows_to_do; /* Amount of temporary space */
    if (verbose) {
      printf("%s: searching for %u new rows\n", name, rows_to_do);
      fflush(stdout);
    }
    for (i = 0; i < rows_to_do; i++) {
      word hash;
      s32 k;
      hash = hash_fn(rows[temp_space + i], hash_len);
      row_vec.hash = hash;
      row_vec.index = temp_space + i;
      found_row = bsearch(&row_vec_ptr, record_ptrs, nor, sizeof(vec *), &hash_compar);
      if (NULL != found_row) {
        /* Maybe an existing row */
        /* Check the file */
        /* First see how many to read */
        int match = 0;
        k = -1;
        /* Check to see if any before this element */
        while (found_row + k >= record_ptrs) {
          if (found_row[k]->hash == hash) {
            k--;
          } else {
            break;
          }
        }
        k++;
        while (found_row + k < record_ptrs + nor) {
          if (found_row[k]->hash == hash) {
            /* Read this row into rows[0] */
            u32 index = found_row[k]->index;
            fseeko64(vectors, ((s64)index) * ((s64)len) * ((s64)sizeof(**rows)), SEEK_SET);
            if (0 == endian_read_row(vectors, *rows, len)) {
              fprintf(stderr, "%s: failed to read row %u from %s, terminating\n",
                      name, index, name_tmp);
              cleanup(NULL, f_a, f_b, vectors);
              (void)remove(name_tmp);
              exit(1);
            }
            /* Check if it matches */
            if (0 == memcmp(*rows,
                            rows[temp_space + i],
                            len * sizeof(**rows))) {
              gen->map[gen->nor + i] = index;
              match = 1;
              break;
            }
          }
          k++;
        }
        if (0 == match) {
          found_row = NULL;
        }
      }
      if (NULL == found_row) {
        /* A new row */
        /* The image of row gen->nor + i under gen is row nor + j */
        word *row;
        word hash = hash_fn(rows[temp_space + i], hash_len);
        /* Swap pointers */
        row = rows[temp_space + j];
        rows[temp_space + j] = rows[temp_space + i];
        rows[temp_space + i] = row;
        hashes[nor + j] = hash;
        gen->map[gen->nor + i] = nor + j;
        row_vec.hash = hash;
        row_vec.index = nor + j;
        records[nor + j] = row_vec;
        j++;
      }
    }
    fseeko64(vectors, end_ptr, SEEK_SET);
    errno = 0;
    if (verbose) {
      printf("%s: adding %u new rows giving %u rows\n", name, j, nor + j);
      fflush(stdout);
    }
    if (0 == endian_write_matrix(vectors, rows + temp_space, len, j)) {
      if ( 0 != errno) {
        perror(name);
      }
      fprintf(stderr, "%s: failed to write %u rows to %s, terminating\n", name, j, name_tmp);
      cleanup(NULL, f_a, f_b, vectors);
      (void)remove(name_tmp);
      exit(1);
    }
    end_ptr = ftello64(vectors);
    /* Note, gen->base_ptr already set */
    gen->nor += rows_to_do;
    nor += j; /* The number of extra rows we made */
    assert(end_ptr == sizeof(**rows) * len * nor);
    /* Now sort the new rows in according to hash */
    /* There's no need to do more than this, because bsearch only uses hash */
    qsort(record_ptrs, nor, sizeof(vec *), &hash_compar);
    gen = gen->next;
  }
  fclose(f_a);
  fclose(f_b);
  if (gen_a.nor < nor || gen_b.nor < nor) {
    fprintf(stderr, "%s: out of memory at %u rows, terminating\n",
            name, nor);
    cleanup(NULL, NULL, NULL, vectors);
    (void)remove(name_tmp);
    exit(2);
  }
  header_set_nor(h_out, nor);
  if (0 == open_and_write_binary_header(&outp, h_out, out, name)) {
    exit(1);
  }
  header_free(h_out);
  /* Copy temp file into output */
  fseeko64(vectors, 0, SEEK_SET);
  errno = 0;
  if (0 == endian_copy_matrix(vectors, outp, *rows, len, nor)) {
    if ( 0 != errno) {
      perror(name);
    }
    fprintf(stderr, "%s: failed to read rows from %s, terminating\n",
            name, name_tmp);
    fclose(outp);
    fclose(vectors);
    (void)remove(name_tmp);
    exit(1);
  }
  fclose(outp);
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
  fclose(vectors);
  (void)remove(name_tmp);
  map_free(map_b);
  header_free(h_map);
  grease_free(&grease);
  free(hashes);
  free(name_tmp);
  return nor;
}
