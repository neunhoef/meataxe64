/*
 * $Id: mspf.c,v 1.17 2004/08/28 19:58:00 jon Exp $
 *
 * Function to spin some vectors under multiple generators
 * using intermediate files in a temporary directory.
 *
 */

#include "mspf.h"
#include "clean_file.h"
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

static void cleanup_all(FILE *f1, unsigned int count, FILE **files,
                        FILE *echelised, const char *name_echelised)
{
  assert(NULL != echelised);
  assert(NULL != name_echelised);
  fclose(echelised);
  (void)remove(name_echelised);
  cleanup(f1, count, files);
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

unsigned int spinf(const char *in, const char *out, const char *dir,
                   unsigned int argc, const char * const args[], const char *name)
{
  FILE *inp = NULL, *outp = NULL, **files = NULL, *echelised = NULL;
  const header *h_in;
  header *h_out;
  char *name_echelised = NULL;
  const char *tmp = tmp_name();
  unsigned int prime, nob, noc, nor, len, max_rows, d, i, clean_nor;
  unsigned int elts_per_word;
  unsigned int **rows1, **rows2;
  int *map;
  grease_struct grease;
  prime_ops prime_operations;
  row_ops row_operations;
  struct gen_struct *gens, *gen;
  assert(NULL != in);
  assert(NULL != out);
  assert(NULL != args);
  assert(NULL != dir);
  assert(0 < argc);
  assert(NULL != name);
  /* Open and examine the vector */
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
    /* Check compatibility */
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
  /* Create names for the temporary files */
  d = strlen(tmp) + strlen(dir);
  name_echelised = my_malloc(d + 4);
  sprintf(name_echelised, "%s/%s.1", dir, tmp);
  /* Initialise arithmetic */
  primes_init(prime, &prime_operations);
  rows_init(prime, &row_operations);
  grease_init(&row_operations, &grease);
  if (0 == grease_level(prime, &grease, memory_rows(len, 100))) {
    fprintf(stderr, "%s: failed to get grease for %s, terminating\n",
            name, in);
    cleanup(inp, argc, files);
    exit(2);
  }
  /* Work out how many rows we can handle in store */
  max_rows = memory_rows(len, 450);
  /* Give up if too few rows available */
  if (max_rows < 2 * (prime + 1)) {
    fprintf(stderr, "%s: failed to get %d rows for %s, %s, terminating\n",
            name, 2 * (prime + 1), in, args[0]);
    cleanup(inp, argc, files);
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
    cleanup(inp, argc, files);
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
      cleanup_all(inp, argc, files, echelised, name_echelised);
      exit(1);
    }
    if (0 == clean_file(&row_operations, echelised, &clean_nor, rows1, stride, rows2, max_rows,
                        map, NULL, 0, grease.level, prime,
                        len, nob, 900, name)) {
      cleanup_all(inp, argc, files, echelised, name_echelised);
      exit(1);
    }
  }
  fclose(inp);
  if (0 == clean_nor) {
    fprintf(stderr, "%s: input set of vectors has rank 0, terminating\n", name);
    cleanup_all(NULL, argc, files, echelised, name_echelised);
  }
  nor = clean_nor;
  /* Set up grease for multiplying */
  if (0 == grease_allocate(prime, len, &grease, 900)){
    fprintf(stderr, "%s: unable to allocate grease, terminating\n", name);
    cleanup_all(NULL, argc, files, echelised, name_echelised);
    exit(1);
  }
  (void)get_mask_and_elts(nob, &elts_per_word);
  while (nor < noc && nor < noc && unfinished(gens, argc, nor)) {
    unsigned int rows_to_do = nor - gen->nor;
    unsigned int k, old_nor = nor;
    /* Ensure we don't try to do too many */
    k = 0;
    while (k < rows_to_do) {
      unsigned int stride = (k + max_rows <= rows_to_do) ? max_rows : rows_to_do - k;
      unsigned int step = (nor > max_rows) ? max_rows : nor;
      unsigned int elt_index = noc, index, i;
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
        cleanup_all(NULL, argc, files, echelised, name_echelised);
        exit(1);
      }
      gen->base_ptr = ftello64(echelised); /* Reset the pointer into the existing basis for this generator */
      /* TODO: convert to use skip_mul */
      for (i = 0; i < stride; i++) {
        int m = map[i + gen->nor];
        assert(m >= 0);
        if ((unsigned int)m < elt_index) {
          elt_index = m;
        }
      }
      index = elt_index / elts_per_word;
      if (0 == skip_mul_from_store(index, rows2, rows1, gen->f, gen->is_map, noc, len, nob,
                                   stride, noc, prime, &grease, verbose, gen->m, name)) {
        fprintf(stderr, "%s: failed to multiply using %s, terminating\n", name, gen->m);
        cleanup_all(NULL, argc, files, echelised, name_echelised);
        exit(1);
      }
      gen->nor += stride;
      d = nor;
      if (verbose) {
        printf("%s: cleaning %d rows\n", name, stride);
        fflush(stdout);
      }
      if (0 == clean_file(&row_operations, echelised, &d, rows1, stride, rows2, step,
                          map, NULL, 0, grease.level, prime,
                          len, nob, 900, name)) {
        cleanup_all(NULL, argc, files, echelised, name_echelised);
        exit(1);
      }
      if (verbose) {
        printf("%s: adding %d new rows giving %d rows\n", name, d - nor, d);
        fflush(stdout);
      }
      nor = d;
      k += stride; /* The number we consumed */
      if (nor >= noc) {
        /* No point in more multiplies if got a full basis */
        break;
      }
    }
    assert(gen->nor == old_nor);
    NOT_USED(old_nor);
    gen = gen->next;
  }
  for (d = 0; d < argc; d++) {
    fclose(files[d]);
  }
  header_set_nor(h_out, nor);
  if (0 == open_and_write_binary_header(&outp, h_out, out, name)) {
    fclose(echelised);
    (void)remove(name_echelised);
    exit(1);
  }
  header_free(h_out);
  fseeko64(echelised, 0, SEEK_SET);
  if (verbose) {
    printf("%s: Copying %d rows to output\n", name, nor);
    fflush(stdout);
  }
  errno = 0;
  if (0 == endian_copy_matrix(echelised, outp, *rows1, len, nor)) {
    if ( 0 != errno) {
      perror(name);
    }
    fprintf(stderr, "%s: failed to read rows from %s, terminating\n",
            name, name_echelised);
    fclose(outp);
    fclose(echelised);
    (void)remove(name_echelised);
    exit(1);
  }
  fclose(outp);
  fclose(echelised);
  free(files);
  free(gens);
  matrix_free(rows1);
  matrix_free(rows2);
  grease_free(&grease);
  free(map);
  (void)remove(name_echelised);
  return nor;
}
