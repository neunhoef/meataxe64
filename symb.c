/*
 * $Id: symb.c,v 1.3 2003/06/13 22:54:10 jon Exp $
 *
 * Function to compute a symmetry basis
 *
 */

#include "symb.h"
#include "clean.h"
#include "clean_file.h"
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
  unsigned int prime, noc, nob, nod, len, nor, rows_available, outer_stride, rows_per_space, total_rows, ech_size, count;
  unsigned int i;
  int *map, *new_map;
  unsigned int **mat, **ech_rows;
  FILE *outp, *temp, **files = NULL, *o;
  const header *h_in;
  header *h_out;
  const char *tmp = tmp_name();
  char *name1, *name2, *name_o1, *name_o2;
  struct gen_struct *gens, *gen;
  grease_struct grease;
  prime_ops prime_operations;
  row_ops row_operations;
  file_struct f, t1, t2;
  file t_in, t_out;
  assert(NULL != in);
  assert(NULL != out);
  assert(NULL != dir);
  i = strlen(tmp) + strlen(dir);
  name1 = my_malloc(i + 4);
  name2 = my_malloc(i + 4);
  name_o1 = my_malloc(i + 4);
  name_o2 = my_malloc(i + 4);
  sprintf(name1, "%s/%s.0", dir, tmp);
  sprintf(name2, "%s/%s.1", dir, tmp); /* Swap between these */
  sprintf(name_o1, "%s/%s.2", dir, tmp); /* For output from the first pass */
  sprintf(name_o2, "%s/%s.3", dir, tmp); /* For output after weeding in the second pass */
  if (0 == spaces || 0 == space_size) {
    fprintf(stderr, "%s: no spaces expected, or zero space size, terminating\n", name);
    exit(1);
  }
  if (0 == open_and_read_binary_header(&f.f, &h_in, in, name)) {
    exit(1);
  }
  prime = header_get_prime(h_in);
  noc = header_get_noc(h_in);
  nor = header_get_nor(h_in);
  len = header_get_len(h_in);
  nob = header_get_nob(h_in);
  nod = header_get_nod(h_in);
  header_free(h_in);
  primes_init(prime, &prime_operations);
  rows_init(prime, &row_operations);
  grease_init(&row_operations, &grease);
  if (0 == grease_level(prime, &grease, memory_rows(len, 100))) {
    fprintf(stderr, "%s: failed to get grease for %s, terminating\n",
            name, in);
    cleanup();
    exit(2);
  }
  if (0 == grease_allocate(prime, len, &grease, 900)){
    fprintf(stderr, "%s: unable to allocate grease, terminating\n", name);
    cleanup();
  }
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
        (nob != header_get_nob(h) && 0 == gens[i].is_map)) {
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
  map = my_malloc((ech_size + space_size * nor) * sizeof(int));
  errno = 0;
  o = fopen64(name_o1, "wb");
  if (NULL == o) {
    if ( 0 != errno) {
      perror(name);
    }
    fprintf(stderr, "%s: failed to open %s, terminating\n", name, name_o1);
    cleanup();
    exit(1);
  }
  /* loop until input all consumed */
  for (i = 0; i < nor; i += outer_stride) {
    /* How many rows this time round */
    unsigned int spaces_per_loop = (i + outer_stride > nor) ? nor - i : outer_stride;
    unsigned int sub_nor = 1;
    unsigned int d;
    /* Remember file pointer into f.f */
    long long ptr = ftello64(f.f);
    /* Read one row into ech_rows */
    if (0 == endian_read_row(f.f, ech_rows[0], len)) {
      if ( 0 != errno) {
        perror(name);
      }
      fprintf(stderr, "%s: failed to read row from %s, terminating\n",
              name, f.name);
      cleanup();
      exit(1);
    }
    /* Compute the map[0] entry, and fail if blank */
    echelise(ech_rows, 1, &d, &new_map, NULL, 0, grease.level, prime, len, nob, 900, 0, 0, 1, name);
    /* Clean up the rows we use for basis detection, 
     * either for multiple rows, or non-identity leading non-zero entry */
    if (0 == d) {
      fprintf(stderr, "%s: read zero row from %s, terminating\n",
              name, f.name);
      cleanup();
      exit(1);
    }
    map[0] = new_map[0];
    free(new_map);
    /* Reset to file pointer and continue */
    fseeko64(f.f, ptr, SEEK_SET);
    gen = gens; /* The first generator */
    t_in = &f; /* Point to correct input and output */
    t_out = &t1;
    while (sub_nor < space_size && unfinished(gens, argc, sub_nor)) {
      unsigned int rows_to_do = sub_nor - gen->nor, new_nor = sub_nor;
      if (0 != rows_to_do) {
        /* Remember file pointer into t_in */
        ptr = ftello64(t_in->f);
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
        fseeko64(t_in->f, ptr, SEEK_SET);
        /* If any new vectors */
        if (0 != d) {
          /* We produce some new rows. Note that this can all be done in memory */
          /* The rows we want are those for which new_map[x] >= 0 */
          unsigned int j, k, l = 0;
          new_nor += d;
          /* Update map with new row info */
          k = sub_nor;
          for (j = 0; j < rows_to_do; j++) {
            if (0 <= new_map[j]) {
              map[k++] = new_map[j];
            }
          }
          assert(sub_nor + d == k);
          for (j = 0; j < spaces_per_loop; j++) {
            /* First ignore the ones we've already done */
            if (0 == endian_read_matrix(t_in->f, mat + l, len, gen->nor)) {
              if ( 0 != errno) {
                perror(name);
              }
              fprintf(stderr, "%s: failed to read rows from %s, terminating\n",
                      name, t_in->name);
              cleanup();
              exit(1);
            }
            /* Now get the ones we want */
            for (k = 0; k < rows_to_do; k++) {
              if (0 == endian_read_row(t_in->f, mat[l], len)) {
                if ( 0 != errno) {
                  perror(name);
                }
                fprintf(stderr, "%s: failed to read rows from %s, terminating\n",
                        name, t_in->name);
                cleanup();
                exit(1);
              }
              /* Do we want this one? */
              if (0 <= new_map[k]) {
                l++;
              }
            }
          }
          /* Compute new rows */
          assert(2 * l <= total_rows);
          if (0 == mul_from_store(mat, mat + l, gen->f, gen->is_map, noc, len, nob,
                                  l, noc, prime, &grease, verbose, gen->m, name)) {
            fprintf(stderr, "%s: failed to multiply using %s, terminating\n", name, gen->m);
            cleanup();
            exit(1);
          }
          errno = 0;
          t_out->f = fopen64(t_out->name, "wb");
          t_out->created = 1;
          if (NULL == t_out->f) {
            if ( 0 != errno) {
              perror(name);
            }
            fprintf(stderr, "%s: failed to open %s, terminating\n", name, t_out->name);
            cleanup();
            exit(1);
          }
          /* Back to start of input rows for this time round loop */
          fseeko64(t_in->f, ptr, SEEK_SET);
          /* Copy old space to new, adding new rows */
          for (j = 0; j < spaces_per_loop; j++) {
            /* Read sub_nor rows from t_in */
            if (0 == endian_read_matrix(t_in->f, mat, len, sub_nor)) {
              if ( 0 != errno) {
                perror(name);
              }
              fprintf(stderr, "%s: failed to read rows from %s, terminating\n",
                      name, t_in->name);
              cleanup();
              exit(1);
            }
            /* write same to t_out */
            if (0 == endian_write_matrix(t_out->f, mat, len, sub_nor)) {
              if ( 0 != errno) {
                perror(name);
              }
              fprintf(stderr, "%s: failed to write rows to %s, terminating\n",
                      name, name_o1);
              cleanup();
              exit(1);
            }
            /* Then write d rows from mat + l to t_out */
            if (0 == endian_write_matrix(t_out->f, mat + l, len, d)) {
              if ( 0 != errno) {
                perror(name);
              }
              fprintf(stderr, "%s: failed to write rows to %s, terminating\n",
                      name, name_o1);
              cleanup();
              exit(1);
            }
            l += d;
          }
          /* Update space pointers */
          if (&f != t_in) {
            fclose(t_in->f);
          }
          fclose(t_out->f);
          t_in = t_in->next;
          t_out = t_out->next;
          errno = 0;
          t_in->f = fopen64(t_in->name, "rb");
          if (NULL == t_in->f) {
            if ( 0 != errno) {
              perror(name);
            }
            fprintf(stderr, "%s: failed to open %s, terminating\n", name, t_in->name);
            cleanup();
            exit(1);
          }
        }
        free(new_map);
      }
      gen->nor = sub_nor;
      sub_nor = new_nor;
      gen = gen->next;
    }
    /* At this point, t_in->f has the vectors */
    if (0 == endian_read_matrix(t_in->f, mat, len, spaces_per_loop * space_size)) {
      if ( 0 != errno) {
        perror(name);
      }
      fprintf(stderr, "%s: failed to read rows from %s, terminating\n",
              name, t_in->name);
      cleanup();
      exit(1);
    }
    if (0 == endian_write_matrix(o, mat, len, spaces_per_loop * space_size)) {
      if ( 0 != errno) {
        perror(name);
      }
      fprintf(stderr, "%s: failed to write rows to %s, terminating\n",
              name, name_o1);
      cleanup();
      exit(1);
    }
    if (&f != t_in) {
      fclose(t_in->f);
    }
  }
  fclose(f.f);
  /* delete name1 and name2 if they exist */
  if (t1.created) {
    (void)remove(t1.name);
  }
  if (t2.created) {
    (void)remove(t2.name);
  }
  fclose(o);
  /* TODO: weed the output from name_o1 to name_o2 */
  /* cf base */
  h_out = header_create(prime, nob, nod, noc, space_size * spaces);
  if (0 == open_and_write_binary_header(&outp, h_out, out, name)) {
    cleanup();
    exit(1);
  }
  header_free(h_out);
  errno = 0;
  f.f = fopen64(name_o1, "rb");
  if (NULL == f.f) {
    if ( 0 != errno) {
      perror(name);
    }
    fprintf(stderr, "%s: cannot open %s, terminating\n", name, name_o1);
    cleanup();
    exit(1);
  }
  errno = 0;
  temp = fopen64(name_o2, "w+b");
  if (NULL == temp) {
    if ( 0 != errno) {
      perror(name);
    }
    fprintf(stderr, "%s: cannot open %s, terminating\n", name, name_o2);
    cleanup();
    exit(1);
  }
  count = 0;
  for (i = 0; i < nor; i++) {
    unsigned int d = count, j;
    if (0 == endian_read_matrix(f.f, mat, len, space_size)) {
      if ( 0 != errno) {
        perror(name);
      }
      fprintf(stderr, "%s: failed to read rows from %s, terminating\n",
              name, name_o1);
      cleanup();
      exit(1);
    }
    /* Now copy to a safe place so the echelisation doesn't corrupt the original */
    for (j = 0; j < space_size; j++) {
      memcpy(ech_rows[j], mat[j], len * sizeof(unsigned int));
    }
    if (0 == clean_file(temp, &count, ech_rows, space_size, mat + space_size, total_rows - space_size,
                        map, NULL, 0, grease.level, prime, len, nob, 900, name)) {
      cleanup();
      exit(1);
    }
    /* Now check the answer */
    if (space_size + d == count) {
      /* Copy */
      if (0 == endian_write_matrix(outp, mat, len, space_size)) {
        if ( 0 != errno) {
          perror(name);
        }
        fprintf(stderr, "%s: failed to write rows to %s, terminating\n",
                name, out);
        cleanup();
        exit(1);
      }
    } else if (count != d) {
      /*Error */
      fprintf(stderr, "%s: unexpected %d rows added, when expecting 0 or %d, terminating\n",
              name, count - d, space_size);
      cleanup();
      exit(1);
    }
  }
  /* Check the size of answer */
  if (space_size * spaces != count) {
    /* Error */
    fprintf(stderr, "%s: unexpected %d spaces found, when expecting %d spaces, terminating\n",
            name, count, spaces);
    cleanup();
  }
  fclose(outp);
  free(map);
  return count;
}
