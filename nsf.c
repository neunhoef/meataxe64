/*
 * $Id: nsf.c,v 1.7 2002/05/26 00:47:20 jon Exp $
 *
 * Compute the nullspace of a matrix, using temporary files
 *
 */

#include "nsf.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "clean.h"
#include "endian.h"
#include "grease.h"
#include "header.h"
#include "ident.h"
#include "matrix.h"
#include "memory.h"
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

unsigned int nullspace(const char *m1, const char *m2, const char *dir, const char *name)
{
  FILE *inp, *outp;
  const header *h;
  header *h_out;
  unsigned int prime, nob, nod, nor, len, len_id, max_len,
      n, r, **mat1, **mat2, **mat3, **mat4,
      i, rows_to_do, max_rows, step1, step2;
  int *map;
  grease_struct grease;
  const char *tmp = tmp_name();
  char *name1, *name2, *name3, *name4, *name5;
  file_struct f, t1, t2;
  file in, out;
  assert(NULL != tmp);
  assert(NULL != dir);
  assert(NULL != m1);
  assert(NULL != m2);
  NOT_USED(mat3);
  NOT_USED(mat4);
  NOT_USED(inp);
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
  outp = fopen64(name5, "wb");
  if (NULL == outp) {
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
  max_len = (len > len_id) ? len : len_id;
  max_rows = memory_rows(max_len, 400);
  r = memory_rows(max_len, 50);
  if (r < prime) {
    fprintf(stderr, "%s: cannot allocate %d rows for %s, terminating\n", name, prime, m1);
    fclose(f.f);
    fclose(f.f_id);
    fclose(outp);
    cleanup(t1, t2, name5);
    exit(2);
  }
  (void)grease_level(prime, &grease, r);
  /* Now read the matrix */
  rows_to_do = nor;
  step1 = (max_rows > nor) ? nor : max_rows;
  step2 = r;
  mat1 = matrix_malloc(step1);
  mat2 = matrix_malloc(step2);
  mat3 = matrix_malloc(step1);
  mat4 = matrix_malloc(step2);
  for (n = 0; n < step1; n++) {
    mat1[n] = memory_pointer_offset(0, n, max_len);
    mat3[n] = memory_pointer_offset(400, n, max_len);
  }
  for (n = 0; n < step2; n++) {
    mat2[n] = memory_pointer_offset(800, n, max_len);
    mat4[n] = memory_pointer_offset(850, n, max_len);
  }
  r = 0; /* Rank count */
  while (rows_to_do > 0) {
    unsigned int rows_remaining = rows_to_do;
    unsigned int stride = (step1 > rows_remaining) ? rows_remaining : step1;
    unsigned int i;
#if 0
    printf("Looping with rank now %d\n", r);
#endif
    if (0 == endian_read_matrix(in->f, mat1, len, stride)) {
      fprintf(stderr, "%s: cannot read matrix for %s, terminating\n", name, in->name);
      fclose(in->f);
      fclose(in->f_id);
      fclose(outp);
      cleanup(t1, t2, name5);
      exit(1);
    }
    if (0 == endian_read_matrix(in->f_id, mat3, len_id, stride)) {
      fprintf(stderr, "%s: cannot read matrix for %s, terminating\n", name, in->name_id);
      fclose(in->f);
      fclose(in->f_id);
      fclose(outp);
      cleanup(t1, t2, name5);
      exit(1);
    }
    echelise(mat1, stride, &n, &map, mat3, 1, grease.level, prime, len, nob, 900, 950, len_id, 1, name);
    rows_remaining -= stride;
    for (i = 0; i < stride; i++) {
      if (map[i] < 0) {
        if (0 == endian_write_row(outp, mat3[i], len_id)) {
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
      if (rows_remaining > 0) {
        unsigned int rows_written = 0;
        out->f = fopen64(out->name, "wb");
        if (NULL == out->f) {
          fprintf(stderr, "%s: cannot open temporary output %s, terminating\n", name, out->name);
          fclose(in->f);
          fclose(in->f_id);
          fclose(outp);
          cleanup(t1, t2, name5);
          exit(1);
        }
        out->f_id = fopen64(out->name_id, "wb");
        out->created = 1;
        if (NULL == out->f_id) {
          fprintf(stderr, "%s: cannot open temporary output %s, terminating\n", name, out->name_id);
          fclose(in->f);
          fclose(in->f_id);
          fclose(out->f);
          fclose(outp);
          cleanup(t1, t2, name5);
          exit(1);
        }
        for (i = 0; i < rows_remaining; i += step2) {
          unsigned int stride2 = (step2 + i > rows_remaining) ? rows_remaining - i : step2;
          unsigned int j;
#if 0
          printf("reading rows %d to %d from %s\n", i, i + stride2, in->name);
#endif
          if (0 == endian_read_matrix(in->f, mat2, len, stride2)) {
            fprintf(stderr, "%s: cannot read matrix for %s, terminating\n", name, in->name);
            fclose(in->f);
            fclose(in->f_id);
            fclose(out->f);
            fclose(out->f_id);
            fclose(outp);
            cleanup(t1, t2, name5);
            exit(1);
          }
          if (0 == endian_read_matrix(in->f_id, mat4, len_id, stride2)) {
            fprintf(stderr, "%s: cannot read matrix for %s, terminating\n", name, in->name_id);
            fclose(in->f);
            fclose(in->f_id);
            fclose(out->f);
            fclose(out->f_id);
            fclose(outp);
            cleanup(t1, t2, name5);
            exit(1);
          }
          clean(mat1, stride, mat2, stride2, map, mat3, mat4, 1, grease.level, prime, len, nob, 900, 950, len_id, name);
          for (j = 0; j < stride2; j++) {
#if 0 /* If we include the code below, we disturb the order of vectors */
/* This invalidates nullspace algorithms for inverses */
            if (0 == row_is_zero(mat2[j], len)) {
#endif
              if (0 == endian_write_row(out->f, mat2[j], len)) {
                fprintf(stderr, "%s: cannot write matrix for %s, terminating\n", name, out->name);
                fclose(in->f);
                fclose(in->f_id);
                fclose(out->f);
                fclose(out->f_id);
                fclose(outp);
                cleanup(t1, t2, name5);
                exit(1);
              }
              if (0 == endian_write_row(out->f_id, mat4[j], len_id)) {
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
#if 0
            } else {
              /* Found a null row, write it out */
              if (0 == endian_write_row(outp, mat4[j], len_id)) {
                fprintf(stderr, "%s: cannot write matrix for %s, terminating\n", name, name5);
                fclose(in->f);
                fclose(in->f_id);
                fclose(out->f);
                fclose(out->f_id);
                fclose(outp);
                cleanup(t1, t2, name5);
                exit(1);
              }
            }
#endif
          }
        }
#if 0
        printf("wrote %d rows to %s\n", rows_written, out->name);
#endif
        fclose(out->f);
        fclose(out->f_id);
        in = in->next;
        out = out->next;
        if (rows_written > 0) {
          in->f = fopen64(in->name, "rb");
          if (NULL == in->f) {
            fprintf(stderr, "%s: cannot open temporary input %s, terminating\n", name, in->name);
            cleanup(t1, t2, name5);
            exit(1);
          }
          in->f_id = fopen64(in->name_id, "rb");
          if (NULL == in->f_id) {
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
    unsigned int *row;
    row = memory_pointer(0);
    h_out = header_create(prime, nob, nod, nor, nor - r);
    if (0 == open_and_write_binary_header(&outp, h_out, m2, name)) {
      fprintf(stderr, "%s: cannot open output %s, terminating\n", name, m2);
      (void)remove(name5);
      exit(1);
    }
    inp = fopen64(name5, "rb");
    if (NULL == inp) {
      fprintf(stderr, "%s: cannot open intermediate null vector file %s, terminating\n",
              name, name5);
      fclose(outp);
      (void)remove(name5);
      exit(1);
    }
    for (i = 0; i < nor - r; i++) {
      if (0 == endian_read_row(inp, row, len_id)) {
        fprintf(stderr, "%s: cannot read row from %s, terminating\n", name, name5);
        fclose(inp);
        fclose(outp);
        (void)remove(name5);
        exit(1);
      }
      if (0 == endian_write_row(outp, row, len_id)) {
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
