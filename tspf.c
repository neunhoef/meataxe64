/*
 * $Id: tspf.c,v 1.19 2005/06/22 21:52:54 jon Exp $
 *
 * Function to spin some vectors under two generators in tensor space
 * using intermediate files in a temporary directory.
 *
 */

#include "tspf.h"
#include "clean_file.h"
#include "elements.h"
#include "endian.h"
#include "grease.h"
#include "header.h"
#include "matrix.h"
#include "memory.h"
#include "mul.h"
#include "mv.h"
#include "parse.h"
#include "primes.h"
#include "read.h"
#include "tra.h"
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
  FILE *f1, *f2;
  const char *m1, *m2;	/* File containing the generator */                       
  u32 nor;	/* Name of the generator */                               
  int is_map1, is_map2;	/* Rows from input already multiplied by this generator */
  word **rows_1;/* Rows of left tensor */
  word **rows_2;/* Rows of right tensor */
  long long base_ptr;	/* Pointer to row nor + 1 in output basis file */
  gen next;		/* Next generator to be used */
};

static void cleanup(FILE *f1, FILE *f2, FILE *f3, FILE *f4, FILE *f5)
{
  if (NULL != f1)
    fclose(f1);
  if (NULL != f2)
    fclose(f2);
  if (NULL != f3)
    fclose(f3);
  if (NULL != f4)
    fclose(f4);
  if (NULL != f5)
    fclose(f5);
}

static void cleanup_tmp(FILE *echelised, const char *name_echelised)
{
  assert(NULL != echelised);
  assert(NULL != name_echelised);
  fclose(echelised);
  (void)remove(name_echelised);
}

u32 tensor_spinf(const char *in, const char *out,
                 const char *a1, const char *a2,
                 const char *b1, const char *b2,
                 const char *dir, const char *name)
{
  FILE *inp = NULL, *outp = NULL, *f_a1 = NULL, *f_a2 = NULL, *f_b1 = NULL, *f_b2 = NULL, *echelised = NULL;
  const header *h_in, *h_a1, *h_a2, *h_b1, *h_b2;
  header *h_out;
  char *name_echelised = NULL;
  const char *tmp = tmp_name();
  u32 prime, nob, noc, nor, noc1, nor1, noc2, nor2, len, len1, len2, len_o, max_rows, d, clean_nor, len_in, size1, size2, limit;
  word **rows1, **rows2, *work_row;
  word **rows_a1, **rows_a2 = NULL, **rows_b1, **rows_b2 = NULL, **mat_rows, **work_rows;
  int *map;
  int all_in_store = 1;
  grease_struct grease;
  prime_ops prime_operations;
  row_ops row_operations;
  struct gen_struct gen_a, gen_b, *gen = &gen_a;
  gen_a.next = &gen_b;
  gen_b.next = &gen_a;
  assert(NULL != in);
  assert(NULL != out);
  assert(NULL != a1);
  assert(NULL != a2);
  assert(NULL != b1);
  assert(NULL != b2);
  assert(NULL != dir);
  assert(NULL != name);
  /* Open and examine the vector and generators */
  if (0 == open_and_read_binary_header(&inp, &h_in, in, name) ||
      0 == open_and_read_binary_header(&f_a1, &h_a1, a1, name) ||
      0 == open_and_read_binary_header(&f_a2, &h_a2, a2, name) ||
      0 == open_and_read_binary_header(&f_b1, &h_b1, b1, name) ||
      0 == open_and_read_binary_header(&f_b2, &h_b2, b2, name)) {
    cleanup(inp, f_a1, f_b1, f_a2, f_b2);
    exit(1);
  }
  prime = header_get_prime(h_in);
  gen_a.is_map1 = 1 == header_get_prime(h_a1);
  gen_b.is_map1 = 1 == header_get_prime(h_b1);
  gen_a.is_map2 = 1 == header_get_prime(h_a2);
  gen_b.is_map2 = 1 == header_get_prime(h_b2);
  nob = header_get_nob(h_in);
  noc = header_get_noc(h_in);
  nor = header_get_nor(h_in);
  len_in = header_get_len(h_in);
  noc1 = header_get_noc(h_a1);
  nor1 = header_get_nor(h_a1);
  noc2 = header_get_noc(h_a2);
  nor2 = header_get_nor(h_a2);
  /* Check all matrices correspond where they should */
  /* a1, b1 to be square and same size, ditto a2 and b2 */
  /* Tensor size of a1, a2 to be # columns in seed vector */
  /* Primes to correspond except for maps */
  if (noc1 != nor1 ||
      noc1 != header_get_noc(h_b1) ||
      noc1 != header_get_nor(h_b1) ||
      noc2 != nor2 ||
      noc2 != header_get_noc(h_b2) ||
      noc2 != header_get_nor(h_b2) ||
      noc1 * noc2 != noc ||
      (prime != header_get_prime(h_a1) && 0 == gen_a.is_map1) ||
      (prime != header_get_prime(h_b1) && 0 == gen_b.is_map1) ||
      (prime != header_get_prime(h_a2) && 0 == gen_a.is_map2) ||
      (prime != header_get_prime(h_b2) && 0 == gen_b.is_map2) ||
      (nob != header_get_nob(h_a1) && 0 == gen_a.is_map1) ||
      (nob != header_get_nob(h_b1) && 0 == gen_b.is_map1) ||
      (nob != header_get_nob(h_a2) && 0 == gen_a.is_map2) ||
      (nob != header_get_nob(h_b2) && 0 == gen_b.is_map2)) {
    fprintf(stderr, "%s: incompatible parameters for %s, %s, %s, %s, %s, terminating\n",
            name, in, a1, b1, a2, b2);
    cleanup(inp, f_a1, f_b1, f_a2, f_b2);
    exit(1);
  }
  /* A place to keep subvector pointers, */
  /* for the benefit of multiply and conversion */
  mat_rows = matrix_malloc(nor1);
  work_rows = matrix_malloc(nor1);
  len1 = header_get_len(h_a1);
  len2 = header_get_len(h_a2);
  len = noc1 * len2; /* Space for matrix form */
  h_out = header_create(prime, nob, header_get_nod(h_in), noc, 1);
  assert(header_get_len(h_out) <= len);
  assert(len_in <= len);
  /* Set up the generator descriptors */
  gen_a.f1 = f_a1;
  gen_a.m1 = a1;
  gen_a.f2 = f_a2;
  gen_a.m2 = a2;
  gen_a.nor = 0;
  gen_a.base_ptr = 0;
  gen_b.f1 = f_b1;
  gen_b.m1 = b1;
  gen_b.f2 = f_b2;
  gen_b.m2 = b2;
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
  size1 = find_extent(nor1, len1);
  size2 = find_extent(nor2, len2);
  if (memory_rows(len1, size1) < nor1 || memory_rows(len2, size2) < nor2 || size1 + size2 > 400) {
    all_in_store = 0;
    if (memory_rows(len1, size1) < nor1 || memory_rows(len2, size2) < nor2 || size1 > 200) {
      fprintf(stderr, "%s: failed to allocate space for one of %s, %s, %s, %s, terminating\n",
              name, a1, b1, a2, b2);
      cleanup(inp, f_a1, f_b1, f_a2, f_b2);
      exit(2);
    }
  }
  limit = (all_in_store) ? 900 - 2 * (size1 + size2) : 900 - 2 * size1;
  rows_a1 = matrix_malloc(nor1);
  rows_b1 = matrix_malloc(nor1);
  if (all_in_store) {
    rows_a2 = matrix_malloc(nor2);
    rows_b2 = matrix_malloc(nor2);
  }
  gen_a.rows_1 = rows_a1;
  gen_b.rows_1 = rows_b1;
  if (all_in_store) {
    gen_a.rows_2 = rows_a2;
    gen_b.rows_2 = rows_b2;
  }
  for (d = 0; d < nor1; d++) {
    rows_a1[d] = memory_pointer_offset(limit, d, len1);
    rows_b1[d] = memory_pointer_offset(limit + size1, d, len1);
  }
  if (all_in_store) {
    for (d = 0; d < nor2; d++) {
      rows_a2[d] = memory_pointer_offset(limit + 2 * size1, d, len2);
      rows_b2[d] = memory_pointer_offset(limit + 2 * size1 + size2, d, len2);
    }
  }
  /* Now compute the maximum space for the subspace */
  max_rows = memory_rows(len, limit / 2);
  /* Give up if too few rows available */
  if (7 > max_rows || max_rows < 2 * (prime + 1)) {
    fprintf(stderr, "%s: failed to alocate enough memory, terminating\n",
            name);
    cleanup(inp, f_a1, f_b1, f_a2, f_b2);
    exit(2);
  }
  max_rows--;
  max_rows = (max_rows > noc) ? noc : max_rows; /* Can never need more than this */
  if (0 == grease_level(prime, &grease, memory_rows(len, 100))) {
    fprintf(stderr, "%s: failed to get grease for %s, %s, %s, %s, %s, terminating\n",
            name, in, a1, b1, a2, b2);
    cleanup(inp, f_a1, f_b1, f_a2, f_b2);
    exit(2);
  }
  /* Set up the pointers to the workspace rows */
  rows1 = matrix_malloc(max_rows);
  rows2 = matrix_malloc(max_rows);
  for (d = 0; d < max_rows; d++) {
    rows1[d] = memory_pointer_offset(0, d + 1, len);
    rows2[d] = memory_pointer_offset(limit / 2, d + 1, len);
  }
  /* Create the temporary file */
  errno = 0;
  echelised = fopen64(name_echelised, "w+b");
  if (NULL == echelised) {
    if ( 0 != errno) {
      perror(name);
    }
    fprintf(stderr, "%s: cannot open %s, terminating\n", name, name_echelised);
    exit(1);
  }
  /* Set up the map for the echelised basis */
  map = my_malloc(noc * sizeof(int));
  clean_nor = 0;
  for (d = 0; d < nor; d += max_rows) {
    u32 stride = (d + max_rows > nor) ? nor - d : max_rows;
    u32 j;
    errno = 0;
    if (0 == endian_read_matrix(inp, rows1, len_in, stride)) {
      if ( 0 != errno) {
        perror(name);
      }
      fprintf(stderr, "%s: failed to read rows from %s, terminating\n",
              name, in);
      cleanup(inp, f_a1, f_b1, f_a2, f_b2);
      cleanup_tmp(echelised, name_echelised);
      exit(1);
    }
    for (j = 0; j < stride; j++) {
      create_pointers(rows2[j], mat_rows, nor1, len2, prime);
      v_to_m(rows1[j], mat_rows, nor1, nor2, prime);
    }
    if (0 == clean_file(&row_operations, echelised, &clean_nor, rows2, stride, rows1, max_rows,
                        map, NULL, 0, grease.level, prime,
                        len, nob, 900, name)) {
      cleanup(inp, f_a1, f_b1, f_a2, f_b2);
      cleanup_tmp(echelised, name_echelised);
      exit(1);
    }
  }
  fclose(inp);
  if (0 == clean_nor) {
    fprintf(stderr, "%s: input set of vectors has rank 0, terminating\n", name);
    cleanup_tmp(echelised, name_echelised);
    cleanup(NULL, f_a1, f_b1, f_a2, f_b2);
  }
  nor = clean_nor;
  /* We reserve memory_pointer_offset(0, 0, len) for workspace */
  work_row = memory_pointer_offset(0, 0, len);
  create_pointers(work_row, work_rows, nor1, len2, prime);
  create_pointers(rows1[0], mat_rows, nor1, len2, prime);
  /* Set up grease for multiplying */
  if (0 == grease_allocate(prime, len, &grease, 900)){
    fprintf(stderr, "%s: unable to allocate grease, terminating\n", name);
    cleanup_tmp(echelised, name_echelised);
    cleanup(NULL, f_a1, f_b1, f_a2, f_b2);
    exit(1);
  }
  /* Think about either being a map */
  if (gen_a.is_map1 || gen_a.is_map2 || gen_b.is_map1 || gen_b.is_map2) {
    fprintf(stderr, "%s: cannot handle maps (yet), terminating\n", name);
    cleanup_tmp(echelised, name_echelised);
    cleanup(NULL, f_a1, f_b1, f_a2, f_b2);
    exit(1);
  }
  /* Read the first group action */
  errno = 0;
  if (0 == endian_read_matrix(f_a1, rows_a1, len1, nor1) ||
      0 == endian_read_matrix(f_b1, rows_b1, len1, nor1)) {
    if ( 0 != errno) {
      perror(name);
    }
    fprintf(stderr, "%s: unable to read %s or %s, terminating\n",
            name, a1, b1);
    cleanup(NULL, f_a1, f_b1, f_a2, f_b2);
    exit(1);
  }
  /* Transpose the first group action */
  tra_in_situ(rows_a1, nor1, nob);
  tra_in_situ(rows_b1, nor1, nob);
  fclose(f_a1);
  fclose(f_b1);
  if (all_in_store) {
    /* Read the second group action */
    errno = 0;
    if (0 == endian_read_matrix(f_a2, rows_a2, len2, nor2) ||
        0 == endian_read_matrix(f_b2, rows_b2, len2, nor2)) {
      if ( 0 != errno) {
        perror(name);
      }
      fprintf(stderr, "%s: unable to read %s or %s, terminating\n",
              name, a2, b2);
      cleanup(NULL, f_a1, f_b1, f_a2, f_b2);
      exit(1);
    }
    fclose(f_a2);
    fclose(f_b2);
  }
  errno = 0;
  while (nor < noc && (gen_a.nor < nor || gen_b.nor < nor)) {
    u32 i, rows_to_do = nor - gen->nor;
    u32 k, old_nor = nor;
    /* Ensure we don't try to do too many */
    k = 0;
    if (verbose) {
      printf("%s: multiplying %d rows\n", name, rows_to_do);
      fflush(stdout);
    }
    while (k < rows_to_do) {
      u32 stride = (k + max_rows <= rows_to_do) ? max_rows : rows_to_do - k;
      u32 step = (nor > max_rows) ? max_rows : nor;
      /* We place the rows to multiply into rows2 */
      /* and produce the product in rows1 */
      /* Seek to correct place in echelised basis */
      fseeko64(echelised, gen->base_ptr, SEEK_SET);
      errno = 0;
      if (0 == endian_read_matrix(echelised, rows2, len, stride)) {
        if ( 0 != errno) {
          perror(name);
        }
        fprintf(stderr, "%s: failed to read %d rows from %s at offset %lld, terminating\n", name, stride, name_echelised, gen->base_ptr);
        cleanup_tmp(echelised, name_echelised);
        exit(1);
      }
      gen->base_ptr = ftello64(echelised); /* Reset the pointer into the existing basis for this generator */
      for (i = 0; i < stride; i++) {
        create_pointers(rows2[i], mat_rows, nor1, len2, prime);
        if (all_in_store) {
          if (0 == mul_in_store(mat_rows, gen->rows_2, work_rows,
                                noc2, len2,
                                nob, nor1, prime,
                                1, &grease)) {
            fprintf(stderr, "%s: failed to multiply using %s, terminating\n", name, gen->m1);
            exit(1);
          }
        } else {
          if (0 == mul_from_store(mat_rows, work_rows, gen->f2, gen->is_map2, noc2, len2, nob,
                                  nor1, noc2, prime, &grease, 0, gen->m2, name)) {
            fprintf(stderr, "%s: failed to multiply using %s, terminating\n", name, gen->m1);
            fclose(f_a2);
            fclose(f_b2);
            exit(1);
          }
        }
        create_pointers(rows1[i], mat_rows, nor1, len2, prime);
        if (0 == mul_in_store(gen->rows_1, work_rows, mat_rows,
                              noc1, len2,
                              nob, nor1, prime,
                              1, &grease)) {
          fprintf(stderr, "%s: failed to multiply using %s, terminating\n", name, gen->m1);
          if (0 == all_in_store) {
            fclose(f_a2);
            fclose(f_b2);
          }
          exit(1);
        }
      }
      gen->nor += stride;
      d = nor;
      if (verbose) {
        printf("%s: cleaning %d rows out of %d for gen %s\n",
               name, stride, rows_to_do, gen->m1);
        fflush(stdout);
      }
      if (0 == clean_file(&row_operations, echelised, &d, rows1, stride, rows2, step,
                          map, NULL, 0, grease.level, prime,
                          len, nob, 900, name)) {
        cleanup_tmp(echelised, name_echelised);
        if (0 == all_in_store) {
          fclose(f_a2);
          fclose(f_b2);
        }
        exit(1);
      }
      if (verbose) {
        printf("%s: adding %d new rows giving %d rows\n", name, d - nor, d);
        fflush(stdout);
      }
      nor = d;
      k += stride; /* The number we consumed */
    }
    assert(gen->nor == old_nor);
    NOT_USED(old_nor);
    gen = gen->next;
  }
  if (0 == all_in_store) {
    fclose(f_a2);
    fclose(f_b2);
  }
  header_set_nor(h_out, nor);
  if (0 == open_and_write_binary_header(&outp, h_out, out, name)) {
    cleanup_tmp(echelised, name_echelised);
    exit(1);
  }
  len_o = header_get_len(h_out);
  header_free(h_in);
  header_free(h_a1);
  header_free(h_b1);
  header_free(h_a2);
  header_free(h_b2);
  header_free(h_out);
  fseeko64(echelised, 0, SEEK_SET);
  if (verbose) {
    printf("%s: Copying %d rows to output\n", name, nor);
    fflush(stdout);
  }
  for (d = 0; d < nor; d++) {
    errno = 0;
    if (0 == endian_read_row(echelised, work_row, len)) {
      if ( 0 != errno) {
        perror(name);
      }
      fprintf(stderr, "%s: failed to read input row %d from %s, terminating\n",
                name, d, name_echelised);
      fclose(outp);
      cleanup_tmp(echelised, name_echelised);
      exit(1);
    }
    m_to_v(work_rows, rows1[0], nor1, noc2, prime);
    errno = 0;
    if (0 == endian_write_row(outp, rows1[0], len_o)) {
      if ( 0 != errno) {
        perror(name);
      }
      fprintf(stderr, "%s: failed to write output row %d to %s, terminating\n",
                name, d, out);
      fclose(outp);
      cleanup_tmp(echelised, name_echelised);
      exit(1);
    }
  }
  fclose(outp);
  cleanup_tmp(echelised, name_echelised);
  free(map);
  matrix_free(rows1);
  matrix_free(rows2);
  grease_free(&grease);
  return nor;
}
