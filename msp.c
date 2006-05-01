/*
 * $Id: msp.c,v 1.18 2006/05/01 09:08:45 jon Exp $
 *
 * Function to spin some vectors under multiple generators
 *
 */

#include "msp.h"
#include "clean.h"
#include "elements.h"
#include "endian.h"
#include "gen.h"
#include "grease.h"
#include "header.h"
#include "matrix.h"
#include "memory.h"
#include "mul.h"
#include "parse.h"
#include "primes.h"
#include "read.h"
#include "utils.h"
#include "write.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>

static void cleanup(FILE *f1, u32 count, FILE **files)
{
  if (NULL != f1) {
    fclose(f1);
  }
  cleanup_files(files, count);
}

u32 spin(const char *in, const char *out,
         unsigned int argc, const char *const args[], const char *name)
{
  FILE *inp = NULL, *outp = NULL, **files = NULL;
  const header *h_in;
  header *h_out;
  u32 prime, nob, noc, nor, len, max_rows, d, j;
  u32 elts_per_word;
  word **rows;
  int *map, *new_map;
  grease_struct grease;
  prime_ops prime_operations;
  row_ops row_operations;
  struct gen_struct *gens, *gen;
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
  gens = my_malloc(argc * sizeof(struct gen_struct));
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
  h_out = header_create(prime, nob, header_get_nod(h_in), noc, nor);
  header_free(h_in);
  primes_init(prime, &prime_operations);
  rows_init(prime, &row_operations);
  grease_init(&row_operations, &grease);
  max_rows = memory_rows(len, 900);
  if (0 == nor) {
    fprintf(stderr, "%s: no rows in input %s, terminating\n", name, in);
    cleanup(inp, argc, files);
    exit(1);
  }
  if (0 == grease_level(prime, &grease, memory_rows(len, 100))) {
    fprintf(stderr, "%s: failed to get grease for %s, terminating\n",
            name, in);
    cleanup(inp, argc, files);
    exit(2);
  }
  rows = matrix_malloc(max_rows);
  for (d = 0; d < max_rows; d++) {
    rows[d] = memory_pointer_offset(0, d, len);
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
  map = my_malloc(max_rows * sizeof(int));
  echelise(&row_operations, rows, nor, &d, &new_map, NULL, 0, grease.level, prime, len, nob, 900, 0, 0, 1, name);
  if (0 == d) {
    fprintf(stderr, "%s: %s contains no non-zero vectors, terminating\n", name, in);
    cleanup(NULL, argc, files);
    exit(1);
  }
  j = 0;
  for (d = 0; d < nor; d++) {
    if (new_map[d] >= 0) {
      word *row;
      map[j] = new_map[d];
      /* Swap pointers */
      row = rows[j];
      rows[j] = rows[d];
      rows[d] = row;
      j++;
    }
  }
  free(new_map);
  nor = j;
  if (0 == grease_allocate(prime, len, &grease, 900)){
    fprintf(stderr, "%s: unable to allocate grease, terminating\n", name);
    cleanup(inp, argc, files);
    exit(1);
  }
  (void)get_mask_and_elts(nob, &elts_per_word);
  while (nor < max_rows && nor < noc && unfinished(gens, argc, nor)) {
    u32 rows_to_do = max_rows - nor;
    u32 i, j = 0;
    u32 elt_index = noc, index;
    /* Ensure we don't try to do too many */
    rows_to_do = (rows_to_do + gen->nor > nor) ? (nor - gen->nor) : rows_to_do;
    for (i = 0; i < rows_to_do; i++) {
      int m = map[i + gen->nor];
      assert(m >= 0);
      if ((u32)m < elt_index) {
        elt_index = m;
      }
    }
    index = elt_index / elts_per_word;
    if (0 == skip_mul_from_store(index, rows + gen->nor, rows + nor, gen->f, gen->is_map, noc, len, nob,
                                 rows_to_do, noc, prime, &grease, verbose, gen->m, name)) {
      fprintf(stderr, "%s: failed to multiply using %s, terminating\n", name, gen->m);
      cleanup(NULL, argc, files);
      exit(1);
    }
    gen->nor += rows_to_do;
    clean(&row_operations, rows, nor, rows + nor, rows_to_do, map, NULL, NULL, 0,
          grease.level, prime, len, nob, 900, 0, 0, verbose, name);
    echelise(&row_operations, rows + nor, rows_to_do, &d, &new_map, NULL, 0,
             grease.level, prime, len, nob, 900, 0, 0, 1, name);
    clean(&row_operations, rows + nor, rows_to_do, rows, nor, new_map, NULL, NULL, 0,
          grease.level, prime, len, nob, 900, 0, 0, 0, name);
    for (i = 0; i < rows_to_do; i++) {
      if (new_map[i] >= 0) {
        /* Got a useful row */
        word *row;
        map[nor + j] = new_map[i];
        /* Swap pointers */
        row = rows[nor + j];
        rows[nor + j] = rows[nor + i];
        rows[nor + i] = row;
        j++;
      }
    }
    free(new_map);
    assert(j == d);
    if (verbose) {
      printf("%s: adding %u new rows giving %u rows for gen %s\n",
             name, d, nor + d, gen->m);
      fflush(stdout);
    }
    nor += d; /* The number of extra rows we made */
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
  if (verbose) {
    printf("%s: Writing %u rows to output\n", name, nor);
    fflush(stdout);
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
  header_free(h_out);
  free(files);
  free(gens);
  matrix_free(rows);
  grease_free(&grease);
  fclose(outp);
  free(map);
  return nor;
}
