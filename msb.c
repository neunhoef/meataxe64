/*
 * $Id: msb.c,v 1.3 2002/07/09 12:05:37 jon Exp $
 *
 * Function to spin some vectors under two generators to obtain a standard base
 *
 */

#include "msb.h"
#include "clean.h"
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
#include "utils.h"
#include "write.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>

typedef struct gen_struct *gen;

struct gen_struct
{
  FILE *f;
  const char *m;
  unsigned int nor;
  int is_map;
  gen next;
};

static void cleanup(FILE *f1, unsigned int count, FILE **files)
{
  if (NULL != f1)
    fclose(f1);
  if (NULL != files) {
    while (count > 0) {
      if (NULL != *files) {
        fclose(*files);
      }
      files++;
      count--;
    }
  }
}

static int unfinished(struct gen_struct *gens,
                      unsigned int argc, unsigned int nor)
{
  while(argc > 0) {
    if (nor > gens[argc - 1].nor) {
      return 1;
    }
    argc--;
  }
  return 0;
}

unsigned int spin(const char *in, const char *out,
                  unsigned int argc, const char * const args[], const char *name)
{
  FILE *inp = NULL, *outp = NULL, **files = NULL;
  const header *h_in;
  header *h_out;
  unsigned int prime, nob, noc, nor, len, max_rows, d;
  unsigned int **rows1, **rows2;
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
        (nob != header_get_nob(h) && 0 == gens[d].is_map) ||
        1 != nor) {
      fprintf(stderr, "%s: incompatible parameters for %s, %s, terminating\n",
              name, in, gen_name);
      cleanup(inp, argc, files);
      exit(1);
    }
    assert(gens[d].is_map || header_get_len(h) == len);
    header_free(h);
  }
  h_out = header_create(prime, nob, header_get_nod(h_in), noc, noc);
  header_free(h_in);
  primes_init(prime, &prime_operations);
  rows_init(prime, &row_operations);
  grease_init(&row_operations, &grease);
  max_rows = memory_rows(len, 450);
  if (0 == grease_level(prime, &grease, memory_rows(len, 100))) {
    fprintf(stderr, "%s: failed to get grease for %s, terminating\n",
            name, in);
    cleanup(inp, argc, files);
    exit(2);
  }
  if (max_rows < noc + 1) {
    fprintf(stderr, "%s: failed to get %d + 1 rows for %s, terminating\n",
            name, noc, in);
    cleanup(inp, argc, files);
    exit(2);
  }
  rows1 = matrix_malloc(max_rows);
  rows2 = matrix_malloc(max_rows);
  for (d = 0; d < max_rows; d++) {
    rows1[d] = memory_pointer_offset(0, d, len);
    rows2[d] = memory_pointer_offset(450, d, len);
  }
  assert(1 == nor);
  errno = 0;
  if (0 == endian_read_matrix(inp, rows1, len, nor)) {
    if ( 0 != errno) {
      perror(name);
    }
    fprintf(stderr, "%s: failed to read rows from %s, terminating\n",
            name, in);
    cleanup(inp, argc, files);
    exit(1);
  }
  fclose(inp);
  assert(1 == nor);
  for (d = 0; d < nor; d++) {
    memcpy(rows2[d], rows1[d], len * sizeof(unsigned int));
  }
  map = my_malloc(max_rows * sizeof(int));
  assert(1 == nor);
  echelise(rows2, nor, &d, &new_map, NULL, 0, grease.level, prime, len, nob, 900, 0, 0, 1, name);
  /* Clean up the rows we use for basis detection, 
   * either for multiple rows, or non-identity leading non-zero entry */
  free(new_map);
  assert(1 == nor);
  if (d != nor) {
    fprintf(stderr, "%s: %s contains dependent vectors, terminating\n", name, in);
    cleanup(NULL, argc, files);
    exit(1);
  }
  assert(1 == nor);
  {
    unsigned int i;
    unsigned int elt = first_non_zero(rows1[0], nob, len, &i);
    assert(0 != elt);
    NOT_USED(elt);
    map[0] = i;
  }
  if (0 == grease_allocate(prime, len, &grease, 900)){
    fprintf(stderr, "%s: unable to allocate grease, terminating\n", name);
    cleanup(NULL, argc, files);
  }
  while (nor < noc && unfinished(gens, argc, nor)) {
    unsigned int rows_to_do = nor - gen->nor;
    unsigned int i, j, k, old_nor = nor;
    /* Ensure we don't try to do too many */
    /* Note that the extra complexity vs sp.c */
    /* is due to the requirement to have a guaranteed */
    /* order of generated vectors, independent of basis or memory constraints */
    k = 0;
    if (verbose) {
      printf("%s: multiplying %d rows\n", name, rows_to_do);
    }
    while (k < rows_to_do) {
      unsigned int rows_poss = max_rows - nor;
      unsigned int stride = (k + rows_poss <= rows_to_do) ? rows_poss : rows_to_do - k;
      if (0 == mul_from_store(rows1 + gen->nor, rows1 + nor, gen->f, gen->is_map, noc, len, nob,
                              stride, noc, prime, &grease, gen->m, name)) {
        fprintf(stderr, "%s: failed to multiply using %s, terminating\n", name, gen->m);
        cleanup(NULL, argc, files);
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
      /* Now we know which rows are new */
      if (nor + d < noc) {
        clean(rows2 + nor, stride, rows2, nor, new_map, NULL, NULL, 0,
              grease.level, prime, len, nob, 900, 0, 0, name);
      } else {
        /* No point in cleaning if we have the whole space */
        /* So force the inner loop to terminate */
        k = rows_to_do - stride;
        gen->nor = old_nor;
      }
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
      if (verbose) {
        printf("%s: Adding %d new rows giving %d rows\n", name, d, nor + d);
      }
      nor += d; /* The number of extra rows we made */
      assert(max_rows + d == rows_poss + nor);
      k += stride;
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
  for (d = 0; d < argc; d++) {
    fclose(files[d]);
  }
  if (nor != noc) {
    fprintf(stderr, "%s: fails to spin to full space (%d, %d), terminating\n",
            name, nor, noc);
    exit(1);
  }
  if (0 == open_and_write_binary_header(&outp, h_out, out, name)) {
    exit(1);
  }
  if (verbose) {
    printf("%s: Writing %d rows to output\n", name, nor);
  }
  errno = 0;
  if (0 == endian_write_matrix(outp, rows1, len, nor)) {
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
  matrix_free(rows1);
  matrix_free(rows2);
  grease_free(&grease);
  fclose(outp);
  free(map);
  return nor;
}
