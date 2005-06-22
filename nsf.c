/*
 * $Id: nsf.c,v 1.19 2005/06/22 21:52:53 jon Exp $
 *
 * Compute the nullspace of a matrix, using temporary files
 *
 */

#include "nsf.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <errno.h>
#include "clean.h"
#include "endian.h"
#include "grease.h"
#include "header.h"
#include "ident.h"
#include "matrix.h"
#include "memory.h"
#include "parse.h"
#include "read.h"
#include "system.h"
#include "utils.h"
#include "write.h"

typedef struct file_struct file_struct, *file;

struct file_struct
{
  FILE *f, *f_id;
  const char *name, *name_id;
  int created;
  file next;
};

static void cleanup(const file_struct t1, const file_struct t2, const char *name5)
{
  if (t1.created) {
    (void)remove(t1.name);
    (void)remove(t1.name_id);
  }
  if (t2.created) {
    (void)remove(t2.name);
    (void)remove(t2.name_id);
  }
  (void)remove(name5);
}

u32 nullspacef(const char *m1, const char *m2, const char *dir, const char *name)
{
  FILE *inp, *outp;
  const header *h;
  header *h_out;
  u32 prime, nob, nod, nor, len, len_id, space, space_id, sub, sub_id,
    n, r, r1, i, rows_to_do, max_rows, step1, step2;
  word **mat1, **mat2, **mat3, **mat4;
  int *map;
  row_ops row_operations;
  grease_struct grease;
  const char *tmp = tmp_name();
  char *name1, *name2, *name3, *name4, *name5;
  file_struct f, t1, t2;
  file in, out;
  assert(NULL != tmp);
  assert(NULL != dir);
  assert(NULL != m1);
  assert(NULL != m2);
  i = strlen(tmp) + strlen(dir);
  name1 = my_malloc(i + 4);
  name2 = my_malloc(i + 4);
  name3 = my_malloc(i + 4);
  name4 = my_malloc(i + 4);
  name5 = my_malloc(i + 4);
  sprintf(name1, "%s/%s.0", dir, tmp);
  sprintf(name2, "%s/%s.1", dir, tmp); /* Two for input */
  sprintf(name3, "%s/%s.2", dir, tmp);
  sprintf(name4, "%s/%s.3", dir, tmp); /* Two for identity */
  sprintf(name5, "%s/%s.4", dir, tmp); /* One for null vectors */
  errno = 0;
  outp = fopen64(name5, "wb");
  if (NULL == outp) {
    if ( 0 != errno) {
      perror(name);
    }
    fprintf(stderr, "%s: cannot open intermediate null vector file %s, terminating\n",
            name, name5);
    exit(1);
  }
  f.name = m1;
  f.name_id = name4;
  t1.name = name1;
  t2.name = name2;
  t1.name_id = name3;
  t2.name_id = name4;
  f.next = &t1;
  t1.next = &t2;
  t2.next = &t1;
  in = &f;
  out = &t1;
  t1.created = 0;
  t2.created = 0;
  if (0 == open_and_read_binary_header(&f.f, &h, m1, name)) {
    cleanup(t1, t2, name5);
    exit(1);
  }
  prime = header_get_prime(h);
  if (1 == prime) {
    fprintf(stderr, "%s: cannot handle maps, terminating\n", name);
    header_free(h);
    fclose(f.f);
    exit(1);
  }
  rows_init(prime, &row_operations);
  nob = header_get_nob(h);
  nod = header_get_nod(h);
  nor = header_get_nor(h);
  len = header_get_len(h);
  header_free(h);
  /* Create an identity */
  if (0 == ident(prime, nor, nor, 1, name4, name)) {
    fclose(f.f);
    cleanup(t1, t2, name5);
    exit(1);
  }
  t2.created = 1;
  if (0 == open_and_read_binary_header(&f.f_id, &h, name4, name)) {
    fclose(f.f);
    fclose(outp);
    cleanup(t1, t2, name5);
    exit(1);
  }
  len_id = header_get_len(h);
  space = split_memory(len, len_id, 1000);
  space_id = 1000 - space;
  if (verbose) {
    printf("%s: splitting memory %u : %u\n", name, space, space_id);
    fflush(stdout);
  }
  assert(10 <= space && space <= 990);
  assert(10 <= space_id && space_id <= 990);
  sub = space / 10;
  sub_id = space_id / 10;
  assert(0 != sub && 0 != sub_id && 2 * sub < space && 2 * sub_id < space_id);
  max_rows = memory_rows(len, space - 2 * sub);
  if (max_rows > memory_rows(len_id, space_id - 2 * sub_id)) {
    max_rows = memory_rows(len_id, space_id - 2 * sub_id);
  }
  r = memory_rows(len, sub);
  r1 = memory_rows(len_id, sub_id);
  if (r1 < r) {
    r = r1; /* For a grease level and step rate compatible with both */
  }
  if (r < prime) {
    fprintf(stderr, "%s: cannot allocate %d rows for %s, terminating\n", name, prime, m1);
    fclose(f.f);
    fclose(f.f_id);
    fclose(outp);
    cleanup(t1, t2, name5);
    exit(2);
  }
  (void)grease_level(prime, &grease, r);
  rows_to_do = nor;
  step1 = (max_rows > nor) ? nor : max_rows;
  step2 = r;
  mat1 = matrix_malloc(step1);
  mat2 = matrix_malloc(step2);
  mat3 = matrix_malloc(step1);
  mat4 = matrix_malloc(step2);
  for (n = 0; n < step1; n++) {
    mat1[n] = memory_pointer_offset(2 * sub, n, len);
    mat3[n] = memory_pointer_offset(space + 2 * sub_id, n, len_id);
  }
  for (n = 0; n < step2; n++) {
    mat2[n] = memory_pointer_offset(sub, n, len);
    mat4[n] = memory_pointer_offset(space + sub_id, n, len_id);
  }
  r = 0; /* Rank count */
  while (rows_to_do > 0) {
    u32 rows_remaining = rows_to_do;
    u32 stride = (step1 > rows_remaining) ? rows_remaining : step1;
    u32 i;
    errno = 0;
    /* Read input rows */
    if (0 == endian_read_matrix(in->f, mat1, len, stride)) {
      if ( 0 != errno) {
        perror(name);
      }
      fprintf(stderr, "%s: cannot read matrix for %s, terminating\n", name, in->name);
      fclose(in->f);
      fclose(in->f_id);
      fclose(outp);
      cleanup(t1, t2, name5);
      exit(1);
    }
    errno = 0;
    /* Read corresponding identity rows */
    if (0 == endian_read_matrix(in->f_id, mat3, len_id, stride)) {
      if ( 0 != errno) {
        perror(name);
      }
      fprintf(stderr, "%s: cannot read matrix for %s, terminating\n", name, in->name_id);
      fclose(in->f);
      fclose(in->f_id);
      fclose(outp);
      cleanup(t1, t2, name5);
      exit(1);
    }
    /* Clean input and record */
    echelise(&row_operations, mat1, stride, &n, &map, mat3, 1, grease.level, prime, len, nob, 0, space, len_id, 1, name);
    rows_remaining -= stride;
    /* Output any new null vectors */
    for (i = 0; i < stride; i++) {
      if (map[i] < 0) {
        errno = 0;
        if (0 == endian_write_row(outp, mat3[i], len_id)) {
          if ( 0 != errno) {
            perror(name);
          }
          fprintf(stderr, "%s: cannot write matrix for %s, terminating\n", name, name5);
          fclose(in->f);
          fclose(in->f_id);
          fclose(outp);
          cleanup(t1, t2, name5);
          exit(1);
        }
      }
    }
    if (0 != n) {
      /* Some addition to the rank */
      r += n;
      /* If still some input rows, then clean with newly echelised rows */
      if (rows_remaining > 0) {
        u32 rows_written = 0;
        errno = 0;
        out->f = fopen64(out->name, "wb");
        if (NULL == out->f) {
          if ( 0 != errno) {
            perror(name);
          }
          fprintf(stderr, "%s: cannot open temporary output %s, terminating\n", name, out->name);
          fclose(in->f);
          fclose(in->f_id);
          fclose(outp);
          cleanup(t1, t2, name5);
          exit(1);
        }
        errno = 0;
        out->f_id = fopen64(out->name_id, "wb");
        out->created = 1;
        if (NULL == out->f_id) {
          if ( 0 != errno) {
            perror(name);
          }
          fprintf(stderr, "%s: cannot open temporary output %s, terminating\n", name, out->name_id);
          fclose(in->f);
          fclose(in->f_id);
          fclose(out->f);
          fclose(outp);
          cleanup(t1, t2, name5);
          exit(1);
        }
        for (i = 0; i < rows_remaining; i += step2) {
          u32 stride2 = (step2 + i > rows_remaining) ? rows_remaining - i : step2;
          u32 j;
          errno = 0;
          if (0 == endian_read_matrix(in->f, mat2, len, stride2)) {
            if ( 0 != errno) {
              perror(name);
            }
            fprintf(stderr, "%s: cannot read matrix for %s, terminating\n", name, in->name);
            fclose(in->f);
            fclose(in->f_id);
            fclose(out->f);
            fclose(out->f_id);
            fclose(outp);
            cleanup(t1, t2, name5);
            exit(1);
          }
          errno = 0;
          if (0 == endian_read_matrix(in->f_id, mat4, len_id, stride2)) {
            if ( 0 != errno) {
               perror(name);
            }
            fprintf(stderr, "%s: cannot read matrix for %s, terminating\n", name, in->name_id);
            fclose(in->f);
            fclose(in->f_id);
            fclose(out->f);
            fclose(out->f_id);
            fclose(outp);
            cleanup(t1, t2, name5);
            exit(1);
          }
          clean(&row_operations, mat1, stride, mat2, stride2, map, mat3, mat4, 1, grease.level, prime, len, nob, 0, space, len_id, verbose, name);
          for (j = 0; j < stride2; j++) {
            errno = 0;
            if (0 == endian_write_row(out->f, mat2[j], len)) {
              if ( 0 != errno) {
                perror(name);
              }
              fprintf(stderr, "%s: cannot write matrix for %s, terminating\n", name, out->name);
              fclose(in->f);
              fclose(in->f_id);
              fclose(out->f);
              fclose(out->f_id);
              fclose(outp);
              cleanup(t1, t2, name5);
              exit(1);
            }
            errno = 0;
            if (0 == endian_write_row(out->f_id, mat4[j], len_id)) {
              if ( 0 != errno) {
                perror(name);
              }
              fprintf(stderr, "%s: cannot write matrix for %s, terminating\n", name, out->name_id);
              fclose(in->f);
              fclose(in->f_id);
              fclose(out->f);
              fclose(out->f_id);
              fclose(outp);
              cleanup(t1, t2, name5);
              exit(1);
            }
            rows_written++;
          }
        }
        fclose(in->f);
        fclose(in->f_id);
        fclose(out->f);
        fclose(out->f_id);
        in = in->next;
        out = out->next;
        if (rows_written > 0) {
          errno = 0;
          in->f = fopen64(in->name, "rb");
          if (NULL == in->f) {
            if ( 0 != errno) {
              perror(name);
            }
            fprintf(stderr, "%s: cannot open temporary input %s, terminating\n", name, in->name);
            cleanup(t1, t2, name5);
            exit(1);
          }
          errno = 0;
          in->f_id = fopen64(in->name_id, "rb");
          if (NULL == in->f_id) {
            if ( 0 != errno) {
              perror(name);
            }
            fprintf(stderr, "%s: cannot open temporary input %s, terminating\n", name, in->name_id);
            cleanup(t1, t2, name5);
            exit(1);
          }
        }
        rows_remaining = rows_written; 
      }
    } else {
      /* Just keep reading from same input */
    }
    rows_to_do = rows_remaining;
    free(map);
  }
  matrix_free(mat1);
  matrix_free(mat2);
  matrix_free(mat3);
  matrix_free(mat4);
  if (t1.created || t2.created) {
    if (t1.created) {
      (void)remove(t1.name);
      (void)remove(t1.name_id);
    }
    if (t2.created) {
      (void)remove(t2.name);
      (void)remove(t2.name_id);
    }
  }
  fclose(outp);
  if (nor > r) {
    /* Found some NULL vectors */
    word *row;
    row = memory_pointer(0);
    h_out = header_create(prime, nob, nod, nor, nor - r);
    if (0 == open_and_write_binary_header(&outp, h_out, m2, name)) {
      (void)remove(name5);
      exit(1);
    }
    errno = 0;
    inp = fopen64(name5, "rb");
    if (NULL == inp) {
      if ( 0 != errno) {
        perror(name);
      }
      fprintf(stderr, "%s: cannot open intermediate null vector file %s, terminating\n",
              name, name5);
      fclose(outp);
      (void)remove(name5);
      exit(1);
    }
    for (i = 0; i < nor - r; i++) {
      errno = 0;
      if (0 == endian_read_row(inp, row, len_id)) {
        if ( 0 != errno) {
          perror(name);
        }
        fprintf(stderr, "%s: cannot read row from %s, terminating\n", name, name5);
        fclose(inp);
        fclose(outp);
        (void)remove(name5);
        exit(1);
      }
      errno = 0;
      if (0 == endian_write_row(outp, row, len_id)) {
        if ( 0 != errno) {
          perror(name);
        }
        fprintf(stderr, "%s: cannot write row to %s, terminating\n", name, m2);
        fclose(inp);
        fclose(outp);
        (void)remove(name5);
        exit(1);
      }
    }
    fclose(inp);
    fclose(outp);
  }
  (void)remove(name5);
  return nor - r;
}
