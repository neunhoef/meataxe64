/*
 * $Id: msbf.c,v 1.13 2004/09/17 17:05:29 jon Exp $
 *
 * Function to spin some vectors under multiple generators to obtain a standard base
 *
 */

#include "msbf.h"
#include "clean.h"
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
  gen next;		/* Next generator to be used */
  int is_map;		/* This generator is a map */
  long long base_ptr;	/* Pointer to row nor + 1 in output basis file */
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
                        FILE *basis, FILE *echelised,
                        const char *name_basis, const char *name_echelised)
{
  assert(NULL != echelised);
  assert(NULL != name_echelised);
  fclose(basis);
  fclose(echelised);
  (void)remove(name_basis);
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

unsigned int msb_spinf(const char *in, const char *out, const char *dir,
                       unsigned int argc, const char * const args[],
                       const char *name)
{
  FILE *inp = NULL, *outp = NULL, **files = NULL, *basis = NULL, *echelised = NULL;
  const header *h_in;
  header *h_out;
  char *name_basis = NULL;
  char *name_echelised = NULL;
  const char *tmp = tmp_name();
  unsigned int prime, nob, noc, nor, len, max_rows, d, i, elt;
  unsigned int **rows1, **rows2, **rows3;
  int *map, *new_map;
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
  /* Check compatibility */
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
  /* Create names for the temporary files */
  d = strlen(tmp) + strlen(dir);
  name_basis = my_malloc(d + 4);
  name_echelised = my_malloc(d + 4);
  sprintf(name_basis, "%s/%s.0", dir, tmp);
  sprintf(name_echelised, "%s/%s.1", dir, tmp);
  /* Initialise arithmetic */
  primes_init(prime, &prime_operations);
  rows_init(prime, &row_operations);
  grease_init(&row_operations, &grease);
  /* Work out how many rows we can handle in store */
  max_rows = memory_rows(len, 300);
  if (0 == grease_level(prime, &grease, memory_rows(len, 100))) {
    fprintf(stderr, "%s: failed to get grease for %s, terminating\n",
            name, in);
    cleanup(inp, argc, files);
    exit(2);
  }
  /* Give up if too few rows available */
  if (max_rows < 2 * (prime + 1)) {
    fprintf(stderr, "%s: failed to get %d rows for %s, terminating\n",
            name, 2 * (prime + 1), in);
    cleanup(inp, argc, files);
    exit(2);
  }
  max_rows = (max_rows > noc) ? noc : max_rows; /* Can never need more than this */
  /* Set up the pointers to the workspace rows */
  rows1 = matrix_malloc(max_rows);
  rows2 = matrix_malloc(max_rows);
  rows3 = matrix_malloc(max_rows);
  for (d = 0; d < max_rows; d++) {
    rows1[d] = memory_pointer_offset(0, d, len);
    rows2[d] = memory_pointer_offset(300, d, len);
    rows3[d] = memory_pointer_offset(600, d, len);
  }
  /* Create the two temporary files */
  errno = 0;
  basis = fopen64(name_basis, "w+b");
  echelised = fopen64(name_echelised, "w+b");
  if (NULL == basis || NULL == echelised) {
    if ( 0 != errno) {
      perror(name);
    }
    fprintf(stderr, "%s: cannot open one of %s or %s, terminating\n", name, name_basis, name_echelised);
    if (NULL != basis) {
      fclose(basis);
      (void)remove(name_basis);
    }
    cleanup(inp, argc, files);
    exit(1);
  }
  assert(1 == nor);
  errno = 0;
  if (0 == endian_read_matrix(inp, rows1, len, 1)) {
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
  memcpy(rows2[0], rows1[0], len * sizeof(unsigned int));
  /* Set up the map for the echelised form of the basis */
  map = my_malloc(noc * sizeof(int));
  /* And compute the first entry */
  assert(1 == nor);
  echelise(&row_operations, rows2, 1, &d, &new_map, NULL, 0, grease.level, prime, len, nob, 900, 0, 0, 1, name);
  /* Clean up the rows we use for basis detection, 
   * in case of non-identity leading non-zero entry */
  free(new_map);
  assert(1 == nor);
  if (1 != d) {
    fprintf(stderr, "%s: %s contains dependent vectors, terminating\n", name, in);
    cleanup_all(NULL, argc, files, basis, echelised, name_basis, name_echelised);
    exit(1);
  }
  new_map = my_malloc(noc * sizeof(int)); /* A map for new rows */
  assert(1 == nor);
  elt = first_non_zero(rows1[0], nob, len, &i);
  assert(0 != elt);
  NOT_USED(elt);
  map[0] = i;
  /* Set up grease for cleaning */
  if (0 == grease_allocate(prime, len, &grease, 900)) {
    fprintf(stderr, "%s: unable to allocate grease, terminating\n", name);
    cleanup_all(NULL, argc, files, basis, echelised, name_basis, name_echelised);
    exit(1);
  }
  errno = 0;
  if (0 == endian_write_row(basis, rows1[0], len) || 0 == endian_write_row(echelised, rows2[0], len)) {
    if ( 0 != errno) {
      perror(name);
    }
    fprintf(stderr, "%s: cannot write initial row to one of %s or %s, terminating\n", name, name_basis, name_echelised);
    cleanup_all(NULL, argc, files, basis, echelised, name_basis, name_echelised);
    exit(1);
  }
  while (nor < noc && unfinished(gens, argc, nor)) {
    unsigned int rows_to_do = nor - gen->nor;
    unsigned int i, k, old_nor = nor;
    /* Ensure we don't try to do too many */
    /* Note that the extra complexity vs sp.c */
    /* is due to the requirement to have a guaranteed */
    /* order of generated vectors, independent of basis or memory constraints */
    k = 0;
    while (k < rows_to_do) {
      unsigned int stride = (k + max_rows <= rows_to_do) ? max_rows : rows_to_do - k;
      unsigned int step = (nor > max_rows) ? max_rows : nor;
      /* We place the rows to multiply into rows2 */
      /* and produce the product in rows1 */
      /* Seek to correct place in basis */
      fseeko64(basis, gen->base_ptr, SEEK_SET);
      errno = 0;
      if (0 == endian_read_matrix(basis, rows2, len, stride)) {
        if ( 0 != errno) {
          perror(name);
        }
        fprintf(stderr, "%s: failed to read %d rows from %s at offset %lld, terminating\n", name, stride, name_basis, gen->base_ptr);
        cleanup_all(NULL, argc, files, basis, echelised, name_basis, name_echelised);
        exit(1);
      }
      gen->base_ptr = ftello64(basis); /* Reset the pointer into the existing basis for this generator */
      if (0 == mul_from_store(rows2, rows1, gen->f, gen->is_map, noc, len, nob,
                              stride, noc, prime, &grease, verbose, gen->m, name)) {
        fprintf(stderr, "%s: failed to multiply using %s, terminating\n", name, gen->m);
        cleanup_all(NULL, argc, files, basis, echelised, name_basis, name_echelised);
        exit(1);
      }
      /* Now copy rows created to rows2, overwriting the rows we multiplied */
      for (i = 0; i < stride; i++) {
        memcpy(rows2[i], rows1[i], len * sizeof(unsigned int));
      }
      gen->nor += stride;
      d = nor;
      if (verbose) {
        printf("%s: cleaning %d rows out of %d for gen %s\n",
               name, stride, rows_to_do, gen->m);
        fflush(stdout);
      }
      if (0 == clean_file(&row_operations, echelised, &d, rows2, stride, rows3, step,
                          map, new_map, 1, grease.level, prime,
                          len, nob, 900, name)) {
        cleanup_all(NULL, argc, files, basis, echelised, name_basis, name_echelised);
        exit(1);
      }
      if (verbose) {
        printf("%s: adding %d new rows giving %d rows\n", name, d - nor, d);
        fflush(stdout);
      }
      nor = d;
      /* Extra code to deal with adding the standard basis vectors */
      fseeko64(basis, 0, SEEK_END);
      for (i = 0; i < stride; i++) {
        if (new_map[i] >= 0) {
          /* Got a useful row */
          errno = 0;
          if (0 == endian_write_row(basis, rows1[i], len)) {
            if (0 != errno) {
              perror(name);
            }
            fprintf(stderr, "%s: failed to write to %s, terminating\n",
                    name, name_basis);
            cleanup_all(NULL, argc, files, basis, echelised, name_basis, name_echelised);
            exit(1);
          }
        }
      }
      k += stride; /* The number we consumed */
      if (nor >= noc) {
        /* No point in more multiplies if got a full basis */
        gen->nor = old_nor;
        break;
      }
    }
    assert(gen->nor == old_nor);
    NOT_USED(old_nor);
    gen = gen->next;
  }
  fclose(echelised);
  (void)remove(name_echelised);
  for (d = 0; d < argc; d++) {
    fclose(files[d]);
  }
  if (nor != noc) {
    fprintf(stderr, "%s: fails to spin to full space (%d, %d), terminating\n",
            name, nor, noc);
    fclose(basis);
    (void)remove(name_basis);
    exit(1);
  }
  if (0 == open_and_write_binary_header(&outp, h_out, out, name)) {
    fclose(basis);
    (void)remove(name_basis);
    exit(1);
  }
  header_free(h_out);
  fseeko64(basis, 0, SEEK_SET);
  if (verbose) {
    printf("%s: Copying %d rows to output\n", name, nor);
    fflush(stdout);
  }
  errno = 0;
  if (0 == endian_copy_matrix(basis, outp, *rows1, len, nor)) {
    if ( 0 != errno) {
      perror(name);
    }
    fprintf(stderr, "%s: failed to read rows from %s, terminating\n",
            name, name_basis);
    fclose(outp);
    fclose(basis);
    (void)remove(name_basis);
    exit(1);
  }
  fclose(outp);
  fclose(basis);
  free(files);
  free(gens);
  matrix_free(rows1);
  matrix_free(rows2);
  matrix_free(rows3);
  free(map);
  free(new_map);
  grease_free(&grease);
  (void)remove(name_basis);
  return nor;
}
