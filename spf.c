/*
 * $Id: spf.c,v 1.8 2002/07/04 22:54:18 jon Exp $
 *
 * Function to spin some vectors under two generators
 *
 */

#include "spf.h"
#include "clean_file.h"
#include "elements.h"
#include "endian.h"
#include "grease.h"
#include "header.h"
#include "matrix.h"
#include "memory.h"
#include "mul.h"
#include "primes.h"
#include "read.h"
#include "system.h"
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
  FILE *f;		/* File containing the generator */
  const char *m;	/* Name of the generator */
  unsigned int nor;	/* Rows from input already multiplied by this generator */
  int is_map;		/* This generator is a map */
  long long base_ptr;	/* Pointer to row nor + 1 in output basis file */
  gen next;		/* Next generator to be used */
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

static void cleanup_all(FILE *f1, FILE *f2, FILE *f3,
                        FILE *echelised, const char *name_echelised)
{
  assert(NULL != echelised);
  assert(NULL != name_echelised);
  fclose(echelised);
  (void)remove(name_echelised);
  cleanup(f1, f2, f3);
}

unsigned int spin(const char *in, const char *out, const char *a,
                  const char *b, const char *dir, const char *name)
{
  FILE *inp = NULL, *outp = NULL, *f_a = NULL, *f_b = NULL, *echelised = NULL;
  const header *h_in, *h_a, *h_b;
  header *h_out;
  char *name_echelised = NULL;
  const char *tmp = tmp_name();
  unsigned int prime, nob, noc, nor, len, max_rows, d, i, clean_nor;
  unsigned int **rows1, **rows2;
  int *map;
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
  assert(NULL != dir);
  assert(NULL != name);
  /* Open and examine the vector and two generators */
  if (0 == open_and_read_binary_header(&inp, &h_in, in, name) ||
      0 == open_and_read_binary_header(&f_a, &h_a, a, name) ||
      0 == open_and_read_binary_header(&f_b, &h_b, b, name)) {
    cleanup(inp, f_a, f_b);
    exit(1);
  }
  /* Check compatibility */
  prime = header_get_prime(h_in);
  gen_a.is_map = 1 == header_get_prime(h_a);
  gen_b.is_map = 1 == header_get_prime(h_b);
  nob = header_get_nob(h_in);
  noc = header_get_noc(h_in);
  nor = header_get_nor(h_in);
  len = header_get_len(h_in);
  if (noc != header_get_noc(h_a) ||
      noc != header_get_nor(h_a) ||
      noc != header_get_noc(h_b) ||
      noc != header_get_nor(h_b) ||
      (prime != header_get_prime(h_a) && 0 == gen_a.is_map) ||
      (prime != header_get_prime(h_b) && 0 == gen_b.is_map) ||
      (nob != header_get_nob(h_a) && 0 == gen_a.is_map) ||
      (nob != header_get_nob(h_b) && 0 == gen_b.is_map)) {
    fprintf(stderr, "%s: incompatible/bad parameters for %s, %s, %s, terminating\n",
            name, in, a, b);
    cleanup(inp, f_a, f_b);
    exit(1);
  }
  assert(gen_a.is_map || header_get_len(h_a) == len);
  assert(gen_b.is_map || header_get_len(h_b) == len);
  h_out = header_create(prime, nob, header_get_nod(h_in), noc, noc);
  header_free(h_in);
  header_free(h_a);
  header_free(h_b);
  /* Set up the generator descriptors */
  gen_a.f = f_a;
  gen_a.m = a;
  gen_a.nor = 0;
  gen_a.base_ptr = 0;
  gen_b.f = f_b;
  gen_b.m = b;
  gen_b.nor = 0;
  gen_b.base_ptr = 0;
  /* Create names for the temporary files */
  d = strlen(tmp) + strlen(dir);
  name_echelised = my_malloc(d + 4);
  sprintf(name_echelised, "%s/%s.1", dir, tmp);
  /* Initialise arithmetic */
  primes_init(prime, &prime_operations);
  rows_init(prime, &row_operations);
  grease_init(&row_operations, &grease);
  /* Work out how many rows we can handle in store */
  max_rows = memory_rows(len, 450);
  if (0 == grease_level(prime, &grease, memory_rows(len, 100))) {
    fprintf(stderr, "%s: failed to get grease for %s, %s, %s, terminating\n",
            name, in, a, b);
    cleanup(inp, f_a, f_b);
    exit(2);
  }
  /* Give up if too few rows available */
  if (max_rows < 2 * (prime + 1)) {
    fprintf(stderr, "%s: failed to get %d rows for %s, %s, %s, terminating\n",
            name, 2 * (prime + 1), in, a, b);
    cleanup(inp, f_a, f_b);
    exit(2);
  }
  max_rows = (max_rows > noc) ? noc : max_rows; /* Can never need more than this */
  /* Set up the pointers to the workspace rows */
  rows1 = matrix_malloc(max_rows);
  rows2 = matrix_malloc(max_rows);
  for (d = 0; d < max_rows; d++) {
    rows1[d] = memory_pointer_offset(0, d, len);
    rows2[d] = memory_pointer_offset(450, d, len);
  }
  /* Create the temporary file */
  errno = 0;
  echelised = fopen64(name_echelised, "w+b");
  if (NULL == echelised) {
    if ( 0 != errno) {
      perror(name);
    }
    fprintf(stderr, "%s: cannot open %s, terminating\n", name, name_echelised);
    cleanup(inp, f_a, f_b);
    exit(1);
  }
  /* Set up the map for the echelised basis */
  map = my_malloc(noc * sizeof(int));
  clean_nor = 0;
  for (i = 0; i < nor; i += max_rows) {
    unsigned int stride = (i + max_rows > nor) ? nor - i : max_rows;
    errno = 0;
    if (0 == endian_read_matrix(inp, rows1, len, stride)) {
      if ( 0 != errno) {
        perror(name);
      }
      fprintf(stderr, "%s: failed to read rows from %s, terminating\n",
              name, in);
      cleanup_all(inp, f_a, f_b, echelised, name_echelised);
      exit(1);
    }
    if (0 == clean_file(echelised, &clean_nor, rows1, stride, rows2, max_rows,
                        map, NULL, 0, grease.level, prime,
                        len, nob, 900, name)) {
      cleanup_all(inp, f_a, f_b, echelised, name_echelised);
      exit(1);
    }
  }
  fclose(inp);
  if (0 == clean_nor) {
    fprintf(stderr, "%s: input set of vectors has rank 0, terminating\n", name);
    cleanup_all(NULL, f_a, f_b, echelised, name_echelised);
  }
  nor = clean_nor;
  /* Set up grease for multiplying */
  if (0 == grease_allocate(prime, len, &grease, 900)) {
    fprintf(stderr, "%s: unable to allocate grease, terminating\n", name);
    cleanup_all(NULL, f_a, f_b, echelised, name_echelised);
  }
  while (nor < noc && (gen_a.nor < nor || gen_b.nor < nor)) {
    unsigned int rows_to_do = nor - gen->nor;
    unsigned int k, old_nor = nor;
    /* Ensure we don't try to do too many */
    k = 0;
    while (k < rows_to_do) {
      unsigned int stride = (k + max_rows <= rows_to_do) ? max_rows : rows_to_do - k;
      unsigned int step = (nor > max_rows) ? max_rows : nor;
      /* We place the rows to multiply into rows2 */
      /* and produce the product in rows1 */
      /* Seek to correct place in echelised basis */
      fseeko64(echelised, gen->base_ptr, SEEK_SET);
      errno = 0;
      if (0 == endian_read_matrix(echelised, rows2, len, stride)) {
        if ( 0 != errno) {
          perror(name);
        }
        fprintf(stderr, "%s: failed to read %d rows from %s at offset %lld, terminating\n", name, stride, gen->m, gen->base_ptr);
        cleanup_all(NULL, f_a, f_b, echelised, name_echelised);
        exit(1);
      }
      gen->base_ptr = ftello64(echelised); /* Reset the pointer into the existing basis for this generator */
      if (0 == mul_from_store(rows2, rows1, gen->f, gen->is_map, noc, len, nob,
                              stride, noc, prime, &grease, gen->m, name)) {
        fprintf(stderr, "%s: failed to multiply using %s, terminating\n", name, gen->m);
        cleanup_all(NULL, f_a, f_b, echelised, name_echelised);
        exit(1);
      }
      gen->nor += stride;
      d = nor;
      if (0 == clean_file(echelised, &d, rows1, stride, rows2, step,
                          map, NULL, 0, grease.level, prime,
                          len, nob, 900, name)) {
        cleanup_all(NULL, f_a, f_b, echelised, name_echelised);
        exit(1);
      }
      nor = d;
      k += stride; /* The number we consumed */
    }
    assert(gen->nor == old_nor);
    NOT_USED(old_nor);
    gen = gen->next;
  }
  fclose(f_a);
  fclose(f_b);
  header_set_nor(h_out, nor);
  if (0 == open_and_write_binary_header(&outp, h_out, out, name)) {
    fclose(echelised);
    (void)remove(name_echelised);
    exit(1);
  }
  header_free(h_out);
  fseeko64(echelised, 0, SEEK_SET);
  copy_rest(outp, echelised);
  fclose(outp);
  fclose(echelised);
  matrix_free(rows1);
  matrix_free(rows2);
  grease_free(&grease);
  free(map);
  (void)remove(name_echelised);
  return nor;
}
