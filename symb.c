/*
 * $Id%
 *
 * Function to compute a symmetry basis
 *
 */

#include "symb.h"
#include "clean.h"
#include "endian.h"
#include "grease.h"
#include "header.h"
#include "matrix.h"
#include "memory.h"
#include "mul.h"
#include "parse.h"
#include "primes.h"
#include "read.h"
#include "rows.h"
#include "system.h"
#include "utils.h"
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

typedef struct file_struct file_struct, *file;

struct file_struct
{
  FILE *f;
  const char *name;
  int created;
  file next;
};

static void cleanup(void)
{
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

unsigned int symb(unsigned int spaces, unsigned int space_size,
                  const char *in, const char *out, const char *dir,
                  unsigned int argc, const char * const args[],
                  const char *name)
{
  unsigned int prime, noc, nob, len, nor, rows_available, outer_stride, rows_per_space, total_rows, ech_size;
  unsigned int i;
  int *map, *new_map;
  unsigned int **mat, **ech_rows;
  FILE *inp, *outp, **files = NULL;
  const header *h_in;
  header *h_out;
  const char *tmp = tmp_name();
  char *name1, *name2;
  struct gen_struct *gens, *gen;
  grease_struct grease;
  prime_ops prime_operations;
  row_ops row_operations;
  file_struct f, t1, t2;
  file t_in, t_out;
  NOT_USED(out);
  NOT_USED(outp);
  NOT_USED(h_out);
  assert(NULL != in);
  assert(NULL != out);
  assert(NULL != dir);
  i = strlen(tmp) + strlen(dir);
  name1 = my_malloc(i + 4);
  name2 = my_malloc(i + 4);
  sprintf(name1, "%s/%s.0", dir, tmp);
  sprintf(name2, "%s/%s.1", dir, tmp); /* Swap between these */
  if (0 == spaces || 0 == space_size) {
    fprintf(stderr, "%s: no spaces expected, or zero space size, terminating\n", name);
    exit(1);
  }
  if (0 == open_and_read_binary_header(&inp, &h_in, in, name)) {
    exit(1);
  }
  prime = header_get_prime(h_in);
  noc = header_get_noc(h_in);
  nor = header_get_nor(h_in);
  len = header_get_len(h_in);
  nob = header_get_nob(h_in);
  primes_init(prime, &prime_operations);
  rows_init(prime, &row_operations);
  grease_init(&row_operations, &grease);
  if (0 == nor || 0 != nor % spaces) {
    fprintf(stderr, "%s: no input rows, or not a multiple of number of spaces\n", name);
    cleanup();
    exit(1);
  }
  /* Memory split */
  /* 10% for grease */
  /* Remainder split 1 : space_size + 1 */
  /* This is enough because we only compute the rows we need, */
  /* except for space 1  which is the predictor */
  rows_available = memory_rows(len, 900);
  rows_per_space = space_size + 2;
  /* Reserve 2 * space_size rows for echelising work */
  ech_size = 2 * space_size;
  outer_stride = (rows_available - ech_size) / rows_per_space;
  if (1 >= outer_stride) {
    fprintf(stderr, "%s: insufficient rows for one space \n", name);
    cleanup();
    exit(1);
  }
  if (outer_stride > nor) {
    /* No need for more rows than we can use */
    outer_stride = nor;
  }
  total_rows = outer_stride * rows_per_space;
  mat = matrix_malloc(total_rows);
  for (i = 0; i < total_rows; i++) {
    mat[i] = memory_pointer_offset(0, i, len);
  }
  ech_rows = matrix_malloc(ech_size);
  for (i = 0; i < ech_size; i++) {
    ech_rows[i] = memory_pointer_offset(0, total_rows + i, len);
  }
  files = my_malloc(argc * sizeof(FILE *));
  gens = my_malloc(argc * sizeof(struct gen_struct));
  for (i = 1; i < argc; i++) {
    gens[i - 1].next = gens + i;
    files[i] = NULL;
  }
  gens[argc - 1].next = gens;
  for (i = 0; i < argc; i++) {
    const char *gen_name = args[i];
    const header *h;
    if (0 == open_and_read_binary_header(files + i, &h, gen_name, name)) {
      cleanup();
      exit(1);
    }
    gens[i].is_map = 1 == header_get_prime(h);
    gens[i].m = gen_name;
    gens[i].f = files[i];
    gens[i].nor = 0;
    if (noc != header_get_noc(h) ||
        noc != header_get_nor(h) ||
        (prime != header_get_prime(h) && 0 == gens[i].is_map) ||
        (nob != header_get_nob(h) && 0 == gens[i].is_map) ||
        (nob != header_get_nob(h) && 0 == gens[i].is_map) ||
        1 != nor) {
      fprintf(stderr, "%s: incompatible parameters for %s, %s, terminating\n",
              name, in, gen_name);
      cleanup();
      exit(1);
    }
    assert(gens[i].is_map || header_get_len(h) == len);
    header_free(h);
  }
  f.name = in;
  t1.name = name1;
  t2.name = name2;
  f.next = &t1;
  t1.next = &t2;
  t2.next = &t1;
  t1.created = 0;
  t2.created = 0;
  map = my_malloc(ech_size * sizeof(int));
  /* TODO: loop until input all consumed */
  for (i = 0; i < nor; i += outer_stride) {
    /* How many rows this time round */
    unsigned int rows_per_loop = (i + outer_stride > nor) ? nor - i : outer_stride;
    unsigned int sub_nor = 1;
    /* TODO: initialise ech_rows and map */
    /* Remember file pointer */
    /* Raed one row into ech_rows */
    /* Compute the map[0] entry, and fail if blank */
    /* Reset to file pointer and continue */
    /* ODOT */
    gen = gens; /* The first generator */
    t_in = &f; /* Point to correct input and output */
    t_out = &t1;
    while (sub_nor < space_size && unfinished(gens, argc, sub_nor)) {
      unsigned int rows_to_do = sub_nor - gen->nor;
      if (0 != rows_to_do) {
        unsigned int d;
        /* Remember file pointer into t_in */
        /* Ignore rows of space 1 we don't want */
        if (0 == endian_read_matrix(t_in->f, mat, len, gen->nor)) {
          if ( 0 != errno) {
            perror(name);
          }
          fprintf(stderr, "%s: failed to read rows from %s, terminating\n",
                  name, t_in->name);
          cleanup();
          exit(1);
        }
        /* Read the rows of space 1 we want */
        if (0 == endian_read_matrix(t_in->f, mat, len, rows_to_do)) {
          if ( 0 != errno) {
            perror(name);
          }
          fprintf(stderr, "%s: failed to read rows from %s, terminating\n",
                  name, t_in->name);
          cleanup();
          exit(1);
        }
        /* Produce the products */
        if (0 == mul_from_store(mat, mat + rows_to_do, gen->f, gen->is_map, noc, len, nob,
                                rows_to_do, noc, prime, &grease, verbose, gen->m, name)) {
          fprintf(stderr, "%s: failed to multiply using %s, terminating\n", name, gen->m);
          cleanup();
          exit(1);
        }
        /* Prune the results */
        clean(ech_rows, sub_nor, mat + rows_to_do, rows_to_do, map, NULL, NULL, 0,
              grease.level, prime, len, nob, 900, 0, 0, verbose, name);
        echelise(mat + rows_to_do, rows_to_do, &d, &new_map, NULL, 0,
                 grease.level, prime, len, nob, 900, 0, 0, 1, name);
        /* Now we know which rows are new */
        if (sub_nor + d < space_size) {
          clean(mat + rows_to_do, rows_to_do, ech_rows, sub_nor, new_map, NULL, NULL, 0,
                grease.level, prime, len, nob, 900, 0, 0, 0, name);
        }
        /* Reset to file pointer in t_in */
        /* If any new vectors */
        if (0 != d) {
          /* Read relevant rows for all spaces */
          /* Compute new rows */
          /* Copy old space to new, adding new rows */
          /* Update space pointer */
        }
        free(new_map);
        NOT_USED(rows_per_loop);
      }
      gen = gen->next;
    }
  }
  /* TODO: weed the output */
  return 0; /* TODO: fix this */
}
