/*
 * $Id: tsp.c,v 1.3 2002/06/28 08:39:16 jon Exp $
 *
 * Function to spin some vectors under two generators in tensor space
 *
 */

#include "tsp.h"
#include "clean.h"
#include "elements.h"
#include "endian.h"
#include "grease.h"
#include "header.h"
#include "matrix.h"
#include "memory.h"
#include "mul.h"
#include "mv.h"
#include "primes.h"
#include "read.h"
#include "tra.h"
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
  const char *m1, *m2;
  unsigned int nor;
  int is_map1, is_map2;
  unsigned int **rows_1;
  unsigned int **rows_2;
  gen next;
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

unsigned int spin(const char *in, const char *out,
                  const char *a1, const char *a2,
                  const char *b1, const char *b2, const char *name)
{
  FILE *inp = NULL, *outp = NULL, *f_a1 = NULL, *f_a2 = NULL, *f_b1 = NULL, *f_b2 = NULL;
  const header *h_in, *h_a1, *h_a2, *h_b1, *h_b2;
  header *h_out;
  unsigned int prime, nob, noc, nor, noc1, nor1, noc2, nor2, len, len1, len2, max_rows, max_nor, max_len, d;
  unsigned int **rows, *work_row;
  unsigned int **rows_a1, **rows_a2, **rows_b1, **rows_b2, **mat_rows, **work_rows;
  int *map, *new_map;
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
  assert(NULL != name);
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
  noc1 = header_get_noc(h_a1);
  nor1 = header_get_nor(h_a1);
  noc2 = header_get_noc(h_a2);
  nor2 = header_get_nor(h_a2);
  max_nor = (nor1 > nor2) ? nor1 : nor2;
  /* Check all matrices correspond where they should */
  /* a1, b1 to be square and same size, ditto a2 and b2 */
  /* Tensor size of a1, a2 to be # columns in seed vector */
  /* Primes to correspond except for maps */
  if (noc1 != header_get_nor(h_a1) ||
      noc1 != header_get_noc(h_b1) ||
      noc1 != header_get_nor(h_b1) ||
      noc2 != header_get_nor(h_a2) ||
      noc2 != header_get_noc(h_b2) ||
      noc2 != header_get_nor(h_b2) ||
      noc1 * noc2 != noc ||
      1 != nor ||
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
  /*for the benefit of multiply and conversion */
  mat_rows = matrix_malloc(nor1);
  work_rows = matrix_malloc(nor1);
  len1 = header_get_len(h_a1);
  len2 = header_get_len(h_a2);
  max_len = (len1 > len2) ? len1 : len2;
  len = noc1 * len2; /* Space for matrix form */
  h_out = header_create(prime, nob, header_get_nod(h_in), noc, 1);
  assert(header_get_len(h_out) <= len);
  assert(header_get_len(h_in) <= len);
  gen_a.f1 = f_a1;
  gen_a.m1 = a1;
  gen_a.f2 = f_a2;
  gen_a.m2 = a2;
  gen_a.nor = 0;
  gen_b.f1 = f_b1;
  gen_b.m1 = b1;
  gen_b.f2 = f_b2;
  gen_b.m2 = b2;
  gen_b.nor = 0;
  primes_init(prime, &prime_operations);
  rows_init(prime, &row_operations);
  grease_init(&row_operations, &grease);
  max_rows = memory_rows(max_len, 50);
  if (max_rows < max_nor) {
    fprintf(stderr, "%s: failed to allocate space for one of %s, %s, %s, %s, terminating\n",
            name, a1, b1, a2, b2);
    cleanup(inp, f_a1, f_b1, f_a2, f_b2);
    exit(2);
  }
  rows_a1 = matrix_malloc(max_nor);
  rows_b1 = matrix_malloc(max_nor);
  rows_a2 = matrix_malloc(max_nor);
  rows_b2 = matrix_malloc(max_nor);
  gen_a.rows_1 = rows_a1;
  gen_b.rows_1 = rows_b1;
  gen_a.rows_2 = rows_a2;
  gen_b.rows_2 = rows_b2;
  for (d = 0; d < max_nor; d++) {
    rows_a1[d] = memory_pointer_offset(700, d, max_len);
    rows_b1[d] = memory_pointer_offset(750, d, max_len);
    rows_a2[d] = memory_pointer_offset(800, d, max_len);
    rows_b2[d] = memory_pointer_offset(850, d, max_len);
  }
  /* Now compute the maximum space for the subspace */
  max_rows = memory_rows(len, 700);
  if (7 > max_rows) {
    fprintf(stderr, "%s: failed to alocate enough memory, terminating\n",
            name);
    cleanup(inp, f_a1, f_b1, f_a2, f_b2);
    exit(2);
  }
  if (0 == grease_level(prime, &grease, memory_rows(len, 100))) {
    fprintf(stderr, "%s: failed to get grease for %s, %s, %s, %s, %s, terminating\n",
            name, in, a1, b1, a2, b2);
    cleanup(inp, f_a1, f_b1, f_a2, f_b2);
    exit(2);
  }
  max_rows--;
  rows = matrix_malloc(max_rows);
  for (d = 0; d < max_rows; d++) {
    rows[d] = memory_pointer_offset(0, d + 1, len);
  }
  /* We reserve memory_pointer_offset(0, 0, len) for workspace */
  work_row = memory_pointer_offset(0, 0, len);
  create_pointers(work_row, work_rows, nor1, len2, prime);
  errno = 0;
  if (0 == endian_read_row(inp, work_row, header_get_len(h_in))) {
    if ( 0 != errno) {
      perror(name);
    }
    fprintf(stderr, "%s: failed to read row from %s, terminating\n", name, in);
    cleanup(inp, f_a1, f_b1, f_a2, f_b2);
    exit(1);
  }
  fclose(inp);
  map = my_malloc(max_rows * sizeof(int));
  create_pointers(rows[0], mat_rows, nor1, len2, prime);
  /* mat_rows subdivides rows[0] as a matrix */
  v_to_m(work_row, mat_rows, nor1, nor2, prime);
  echelise(rows, 1, &d, &new_map, NULL, 0, grease.level, prime, len, nob, 900, 0, 0, 1, name);
  /* Clean up either for zero row, or non-identity leading non-zero entry */
  free(new_map);
  if (1 != d) {
    fprintf(stderr, "%s: %s contains dependent vectors, terminating\n", name, in);
    cleanup(NULL, f_a1, f_b1, f_a2, f_b2);
    exit(1);
  }
  {
    unsigned int i;
    unsigned int elt = first_non_zero(rows[0], nob, len, &i);
    assert(0 != elt);
    NOT_USED(elt);
    map[0] = i;
  }
  if (0 == grease_allocate(prime, len, &grease, 900)){
    fprintf(stderr, "%s: unable to allocate grease, terminating\n", name);
    cleanup(NULL, f_a1, f_b1, f_a2, f_b2);
    exit(1);
  }
  /* Think about either being a map */
  if (gen_a.is_map1 || gen_a.is_map2 || gen_b.is_map1 || gen_b.is_map2) {
    fprintf(stderr, "%s: cannot handle maps (yet), terminating\n", name);
    cleanup(NULL, f_a1, f_b1, f_a2, f_b2);
    exit(1);
  }
  /* Read the first group action */
  errno = 0;
  if (0 == endian_read_matrix(f_a1, rows_a2, len1, nor1) ||
      0 == endian_read_matrix(f_b1, rows_b2, len1, nor1)) {
    if ( 0 != errno) {
      perror(name);
    }
    fprintf(stderr, "%s: unable to read %s or %s, terminating\n",
            name, a1, b1);
    cleanup(NULL, f_a1, f_b1, f_a2, f_b2);
    exit(1);
  }
  /* Transpose the first group action */
  tra_in_store(rows_a2, rows_a1, nor1, noc1, nob, len1);
  tra_in_store(rows_b2, rows_b1, nor1, noc1, nob, len1);
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
  fclose(f_a1);
  fclose(f_b1);
  fclose(f_a2);
  fclose(f_b2);
  while (nor < max_rows && nor < noc && (gen_a.nor < nor || gen_b.nor < nor)) {
    unsigned int rows_to_do = max_rows - nor;
    unsigned int i, j = 0;
    /* Ensure we don't try to do too many */
    rows_to_do = (rows_to_do + gen->nor > nor) ? (nor - gen->nor) : rows_to_do;
    for (i = 0; i < rows_to_do; i++) {
      create_pointers(rows[gen->nor + i], mat_rows, nor1, len2, prime);
      if (0 == mul_in_store(mat_rows, gen->rows_2, work_rows,
                            0, gen->is_map2, noc2, len2,
                            nob, nor1, noc2, prime,
                            &grease, gen->m1, gen->m2, name)) {
        fprintf(stderr, "%s: failed to multiply using %s, terminating\n", name, gen->m1);
        exit(1);
      }
      create_pointers(rows[nor + i], mat_rows, nor1, len2, prime);
      if (0 == mul_in_store(gen->rows_1, work_rows, mat_rows,
                            gen->is_map1, 0, noc1, len2,
                            nob, nor1, noc2, prime,
                            &grease, gen->m1, gen->m2, name)) {
        fprintf(stderr, "%s: failed to multiply using %s, terminating\n", name, gen->m1);
        exit(1);
      }
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
        unsigned int *row;
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
    nor += d; /* The number of extra rows we made */
    gen = gen->next;
  }
  if (nor >= max_rows) {
    fprintf(stderr, "%s: out of memory at %d rows, terminating\n",
            name, nor);
    exit(2);
  }
  header_set_nor(h_out, nor);
  if (0 == open_and_write_binary_header(&outp, h_out, out, name)) {
    exit(1);
  }
  len = header_get_len(h_out);
  for (d = 0; d < nor; d++) {
    create_pointers(rows[d], mat_rows, nor1, len2, prime);
    m_to_v(mat_rows, work_row, nor1, noc2, prime);
    errno = 0;
    if (0 == endian_write_row(outp, work_row, len)) {
      if ( 0 != errno) {
        perror(name);
      }
      fprintf(stderr, "%s: failed to write output row %d to %s, terminating\n",
              name, d, out);
      fclose(outp);
      exit(1);
    }
  }
  header_free(h_in);
  header_free(h_a1);
  header_free(h_b1);
  header_free(h_a2);
  header_free(h_b2);
  header_free(h_out);
  matrix_free(rows);
  grease_free(&grease);
  fclose(outp);
  free(map);
  return nor;
}
