/*
 * $Id: rnf.c,v 1.11 2002/06/28 08:39:16 jon Exp $
 *
 * Compute the rank of a matrix, using temporary files
 *
 */

#include "rnf.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <errno.h>
#include "clean.h"
#include "endian.h"
#include "grease.h"
#include "header.h"
#include "maps.h"
#include "matrix.h"
#include "memory.h"
#include "read.h"
#include "system.h"
#include "utils.h"
#include "write.h"

typedef struct file_struct file_struct, *file;

struct file_struct
{
  FILE *f;
  const char *name;
  int created;
  file next;
};

static void cleanup(const file_struct t1, const file_struct t2)
{
  if (t1.created) {
    (void)remove(t1.name);
  }
  if (t2.created) {
    (void)remove(t2.name);
  }
}

static void cleanup_outp(int record, FILE *outp, const char *name)
{
  assert(NULL != outp);
  assert(NULL != name);
  if (record) {
    fclose(outp);
    (void)remove(name);
  }
}

unsigned int rank(const char *m1, const char *dir, const char *m2,
                  int record, int full, const char *name)
{
  FILE *inp, *outp;
  const header *h;
  header *h_out;
  unsigned int prime, nob, nod, noc, nor, len, n, r, **mat1, **mat2, i, rows_to_do, max_rows, step1, step2;
  int *map;
  grease_struct grease;
  const char *tmp = tmp_name();
  char *name1, *name2, *name3 = NULL;
  file_struct f, t1, t2;
  file in, out;
  assert(NULL != tmp);
  assert(NULL != dir);
  assert(NULL != m1);
  assert((0 == record && NULL == m2) ||
         (0 != record && NULL != m2));
  NOT_USED(full);
  i = strlen(tmp) + strlen(dir);
  name1 = my_malloc(i + 4);
  name2 = my_malloc(i + 4);
  sprintf(name1, "%s/%s.0", dir, tmp);
  sprintf(name2, "%s/%s.1", dir, tmp);
  if (0 == record) {
    NOT_USED(name3);
    NOT_USED(outp);
    full = 0; /* No point in back cleaning if we don't record */
  } else {
    name3 = my_malloc(i + 4);
    sprintf(name3, "%s/%s.2", dir, tmp);
  }
  f.name = m1;
  t1.name = name1;
  t2.name = name2;
  f.next = &t1;
  t1.next = &t2;
  t2.next = &t1;
  in = &f;
  out = &t1;
  t1.created = 0;
  t2.created = 0;
  if (0 == open_and_read_binary_header(&inp, &h, m1, name)) {
    exit(1);
  }
  f.f = inp;
  prime = header_get_prime(h);
  if (1 == prime) {
    if (0 == map_rank(inp, h, m1, &r, name)) {
      fclose(inp);
      header_free(h);
      exit(1);
    }
    header_free(h);
    fclose(inp);
    return r;
  }
  nob = header_get_nob(h);
  nor = header_get_nor(h);
  nod = header_get_nod(h);
  noc = header_get_noc(h);
  len = header_get_len(h);
  header_free(h);
  max_rows = memory_rows(len, 800);
  r = memory_rows(len, 100);
  if (r < prime) {
    fprintf(stderr, "%s: cannot allocate %d rows for %s, terminating\n", name, prime, m1);
    fclose(inp);
    exit(2);
  }
  (void)grease_level(prime, &grease, r);
  /* Now read the matrix */
  rows_to_do = nor;
  step1 = (max_rows > nor) ? nor : max_rows;
  step2 = r;
  mat1 = matrix_malloc(step1);
  mat2 = matrix_malloc(step2);
  for (n = 0; n < step1; n++) {
    mat1[n] = memory_pointer_offset(0, n, len);
  }
  for (n = 0; n < step2; n++) {
    mat2[n] = memory_pointer_offset(800, n, len);
  }
  r = 0; /* Rank count */
  if (0 != record) {
    errno = 0;
    outp = fopen64(name3, "wb");
    if (NULL == outp) {
      if ( 0 != errno) {
        perror(name);
      }
      fprintf(stderr, "%s: failed to open temporary file %s, terminating\n", name, name3);
      fclose(inp);
      exit(1);
    }
  }
  while (rows_to_do > 0) {
    unsigned int rows_remaining = rows_to_do;
    unsigned int stride = (step1 > rows_remaining) ? rows_remaining : step1;
    errno = 0;
    if (0 == endian_read_matrix(in->f, mat1, len, stride)) {
      if ( 0 != errno) {
        perror(name);
      }
      fprintf(stderr, "%s: cannot read matrix for %s, terminating\n", name, in->name);
      fclose(in->f);
      cleanup_outp(record, outp, name3);
      cleanup(t1, t2);
      exit(1);
    }
    echelise(mat1, stride, &n, &map, NULL, 0, grease.level, prime, len, nob, 900, 0, 0, 1, name);
    rows_remaining -= stride;
    if (0 != n) {
      /* Some addition to the rank */
      r += n;
      if (0 != record) {
        for (i = 0; i < stride; i++) {
          if (map[i] >= 0) {
            errno = 0;
            if (0 == endian_write_row(outp, mat1[i], len)) {
              if ( 0 != errno) {
                perror(name);
              }
              fprintf(stderr, "%s: cannot write row to %s, terminating\n", name, name3);
              fclose(in->f);
              cleanup_outp(record, outp, name3);
              cleanup(t1, t2);
              exit(1);
            }
          }
        }
      }
      if (rows_remaining > 0) {
        unsigned int rows_written = 0;
        errno = 0;
        out->f = fopen64(out->name, "wb");
        out->created = 1;
        if (NULL == out->f) {
          if ( 0 != errno) {
            perror(name);
          }
          fprintf(stderr, "%s: cannot open temporary output %s, terminating\n", name, out->name);
          fclose(in->f);
          cleanup_outp(record, outp, name3);
          cleanup(t1, t2);
          exit(1);
        }
        for (i = 0; i < rows_remaining; i += step2) {
          unsigned int stride2 = (step2 + i > rows_remaining) ? rows_remaining - i : step2;
          unsigned int j;
          errno = 0;
          if (0 == endian_read_matrix(in->f, mat2, len, stride2)) {
            if ( 0 != errno) {
              perror(name);
            }
            fprintf(stderr, "%s: cannot read matrix for %s, terminating\n", name, in->name);
            fclose(in->f);
            fclose(out->f);
            cleanup_outp(record, outp, name3);
            cleanup(t1, t2);
            exit(1);
          }
          clean(mat1, stride, mat2, stride2, map, NULL, NULL, 0, grease.level, prime, len, nob, 900, 0, 0, name);
          for (j = 0; j < stride2; j++) {
            if (0 == row_is_zero(mat2[j], len)) {
              errno = 0;
              if (0 == endian_write_row(out->f, mat2[j], len)) {
                if ( 0 != errno) {
                  perror(name);
                }
                fprintf(stderr, "%s: cannot write matrix for %s, terminating\n", name, out->name);
                fclose(in->f);
                fclose(out->f);
                cleanup_outp(record, outp, name3);
                cleanup(t1, t2);
                exit(1);
              }
              rows_written++;
            }
          }
        }
        fclose(out->f);
        in = in->next;
        out = out->next;
        if (rows_written > 0) {
          errno = 0;
          in->f = fopen64(in->name, "rb");
          if (NULL == in->f) {
            if ( 0 != errno) {
              perror(name);
            }
            fprintf(stderr, "%s: cannot open temporary output %s, terminating\n", name, in->name);
            cleanup_outp(record, outp, name3);
            cleanup(t1, t2);
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
  cleanup(t1, t2);
  if (0 != record) {
    unsigned int *row;
    row = memory_pointer(0);
    fclose(outp);
    h_out = header_create(prime, nob, nod, noc, r);
    if (0 == open_and_write_binary_header(&outp, h_out, m2, name)) {
      exit(1);
    }
    errno = 0;
    inp = fopen64(name3, "rb");
    if (NULL == inp) {
      if ( 0 != errno) {
        perror(name);
      }
      fclose(outp);
      fprintf(stderr, "%s: cannot open input %s, terminating\n",name, name3);
      exit(1);
    }
    for (i = 0; i < r; i++) {
      errno = 0;
      if (0 == endian_read_row(inp, row, len)) {
        if ( 0 != errno) {
          perror(name);
        }
        fprintf(stderr, "%s: cannot read row from %s, terminating\n", name, name3);
        fclose(inp);
        fclose(outp);
        exit(1);
      }
      errno = 0;
      if (0 == endian_write_row(outp, row, len)) {
        if ( 0 != errno) {
          perror(name);
        }
        fprintf(stderr, "%s: cannot write row to %s, terminating\n", name, m2);
        fclose(inp);
        fclose(outp);
        exit(1);
      }
    }
    fclose(inp);
    fclose(outp);
    (void)remove(name3);
  }
  return r;
}
