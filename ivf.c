/*
 * $Id: ivf.c,v 1.6 2003/02/28 20:04:58 jon Exp $
 *
 * Invert a matrix using intermediate files
 *
 */

#include "ivf.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include "clean.h"
#include "elements.h"
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

static void cleanup_all(FILE *echelised, const char *name_echelised,
                        FILE *id, const char *name_id)
{
  assert(NULL != name_echelised);
  assert(NULL != name_id);
  if (NULL != echelised) {
    fclose(echelised);
    (void)remove(name_echelised);
  }
  if (NULL != id) {
    fclose(id);
    (void)remove(name_id);
  }
}

void invert(const char *m1, const char *m2, const char *dir, const char *name)
{
  FILE *inp, *outp;
  const header *h;
  unsigned int prime, nob, nor, len, n, r;
  int *map1;
  int is_perm;
  grease_struct grease;
  assert(NULL != m1);
  assert(NULL != m2);
  assert(NULL != dir);
  assert(NULL != name);
  if (0 == open_and_read_binary_header(&inp, &h, m1, name)) {
    exit(1);
  }
  prime = header_get_prime(h);
  is_perm = 1 == prime;
  nob = header_get_nob(h);
  nor = header_get_nor(h);
  if (header_get_noc(h) != nor) {
    fprintf(stderr, "%s: matrix %s is not square, terminating\n", name, m1);
    fclose(inp);
    exit(1);
  }
  if (is_perm) {
    NOT_USED(dir);
    if (0 == map_iv(inp, h, m1, m2, name)) {
      fclose(inp);
      header_free(h);
      exit(1);
    }
    fclose(inp);
    header_free(h);
  } else {
    const char *tmp = tmp_name();
    int tmps_created = 0;
    char *name_echelised = NULL, *name_id = NULL;
    FILE *echelised = NULL, *id = NULL;
    unsigned int max_rows, work_rows;
    /* rows1, rows2 for area being worked on */
    /* rows3, rows4 for forward and backward cleaning */
    unsigned int **rows1, **rows2, **rows3, **rows4;
    long long ptr_echelised = 0, ptr_id = 0;
    /* Create names for the temporary files */
    n = strlen(tmp) + strlen(dir);
    name_echelised = my_malloc(n + 4);
    name_id = my_malloc(n + 4);
    sprintf(name_echelised, "%s/%s.1", dir, tmp);
    sprintf(name_id, "%s/%s.2", dir, tmp);
    len = header_get_len(h);
    work_rows = memory_rows(len, 50);
    /* We need at least prime rows for grease */
    /* And at least two rows to work on, but we know max_rows >= r */
    max_rows = memory_rows(len, 400);
    if (work_rows < prime) {
      fprintf(stderr, "%s: cannot allocate %d rows for %s and %s, terminating\n",
              name, 2 * (2 + prime), m1, m2);
      fclose(inp);
      header_free(h);
      exit(2);
    }
    max_rows = (max_rows > nor) ? nor : max_rows;
    /* Set up the row pointers */
    rows1 = matrix_malloc(max_rows);
    rows2 = matrix_malloc(max_rows);
    rows3 = matrix_malloc(work_rows);
    rows4 = matrix_malloc(work_rows);
    for (n = 0; n < max_rows; n++) {
      rows1[n] = memory_pointer_offset(0, n, len);
      rows2[n] = memory_pointer_offset(400, n, len);
    }
    for (n = 0; n < work_rows; n++) {
      rows3[n] = memory_pointer_offset(800, n, len);
      rows4[n] = memory_pointer_offset(850, n, len);
    }
    (void)grease_level(prime, &grease, work_rows);
    r = grease.level;
    errno = 0;
    echelised = fopen64(name_echelised, "w+b");
    id = fopen64(name_id, "w+b");
    if (NULL == echelised || NULL == id) {
      if ( 0 != errno) {
        perror(name);
      }
      fprintf(stderr, "%s: cannot open %s, terminating\n", name, name_echelised);
      cleanup_all(echelised, name_echelised, id, name_id);
      fclose(inp);
      header_free(h);
      exit(1);
    }
    tmps_created = 1;
    /* Initialise id */
    for (n = 0; n < nor; n++) {
      row_init(rows2[0], len);
      put_element_to_row(nob, n, rows2[0], 1);
      errno = 0;
      if (0 == endian_write_row(id, rows2[0], len)) {
        if ( 0 != errno) {
          perror(name);
        }
        fprintf(stderr, "%s: cannot write row %d to %s, terminating\n", name, n, name_id);
        fclose(inp);
        header_free(h);
        cleanup_all(echelised, name_echelised, id, name_id);
        exit(1);
      }
    }
    /* Copy input to echelised */
    errno = 0;
    if (0 == endian_copy_matrix(inp, echelised, *rows1, len, nor)) {
      if ( 0 != errno) {
        perror(name);
      }
      fprintf(stderr, "%s: failed to read rows from %s, terminating\n",
              name, m1);
      fclose(inp);
      header_free(h);
      cleanup_all(echelised, name_echelised, id, name_id);
      exit(1);
    }
    fclose(inp);
    map1 = my_malloc(nor * sizeof(unsigned int));
    /* Back to start of echelised and id */
    fseeko64(echelised, 0, SEEK_SET);
    fseeko64(id, 0, SEEK_SET);
    for (r = 0; r < nor; r += max_rows) {
      /* Now read the two submatrices */
      unsigned int stride = (r + max_rows <= nor) ? max_rows : nor - r;
      long long ptr_e1 = ftello64(echelised), ptr_i1 = ftello64(id);
      int *map;
      errno = 0;
      if (0 == endian_read_matrix(echelised, rows1, len, stride)) {
        if ( 0 != errno) {
          perror(name);
        }
        fprintf(stderr, "%s: cannot read %d rows from matrix for %s, terminating\n", name, stride, name_echelised);
        header_free(h);
        cleanup_all(echelised, name_echelised, id, name_id);
        exit(1);
      }
      errno = 0;
      if (0 == endian_read_matrix(id, rows2, len, stride)) {
        if ( 0 != errno) {
          perror(name);
        }
        fprintf(stderr, "%s: cannot read %d rows from matrix for %s, terminating\n", name, stride, name_id);
        header_free(h);
        cleanup_all(echelised, name_echelised, id, name_id);
        exit(1);
      }
      ptr_echelised = ftello64(echelised);
      ptr_id = ftello64(id);
      echelise(rows1, stride, &n, &map, rows2, 1, grease.level, prime, len, nob, 900, 950, len, 1, name);
      if (stride != n) {
        fprintf(stderr, "%s: matrix %s is singular with rank at most %d, terminating\n", name, m1, nor + n - stride);
        header_free(h);
        cleanup_all(echelised, name_echelised, id, name_id);
        exit(1);
      }
      /* Copy the new map into the right place in the existing one */
      memcpy(map1 + r, map, n * sizeof(unsigned int));
      /* Now write back the cleaning result */
      fseeko64(echelised, ptr_e1, SEEK_SET);
      fseeko64(id, ptr_i1, SEEK_SET);
      errno = 0;
      if (0 == endian_write_matrix(echelised, rows1, len, stride) ||
          0 == endian_write_matrix(id, rows2, len, stride)) {
        if ( 0 != errno) {
          perror(name);
        }
        fprintf(stderr, "%s: failed to write %d rows to one of %s or %s, terminating\n",
                name, stride, name_echelised, name_id);
        header_free(h);
        cleanup_all(echelised, name_echelised, id, name_id);
        exit(1);
      }
      /* Now back clean the earlier stuff */
      ptr_e1 = 0; /* where we are in echelised */
      ptr_i1 = 0; /* where we are in id */
      for (n = 0; n < r; n += work_rows) {
        unsigned int stride2 = (work_rows + n > r) ? r - n : work_rows;
        /* Read stride2 rows from echelised and id at offsets ptr_e1, ptr_i1 */
        fseeko64(echelised, ptr_e1, SEEK_SET);
        fseeko64(id, ptr_i1, SEEK_SET);
        errno = 0;
        if (0 == endian_read_matrix(echelised, rows3, len, stride2) ||
            0 == endian_read_matrix(id, rows4, len, stride2)) {
          if ( 0 != errno) {
            perror(name);
          }
          fprintf(stderr, "%s: cannot read %d rows from one of %s or %s, terminating\n",
                  name, stride2, name_echelised, name_id);
          header_free(h);
          cleanup_all(echelised, name_echelised, id, name_id);
        }
        /* Clean the rows we read with the newly made rows */
        clean(rows1, stride, rows3, stride2, map1 + r, rows2, rows4, 1,
              grease.level, prime, len, nob, 900, 950, len, 0, name);
        /* Write back the cleaned version to echelised and id */
        fseeko64(echelised, ptr_e1, SEEK_SET);
        fseeko64(id, ptr_i1, SEEK_SET);
        errno = 0;
        if (0 == endian_write_matrix(echelised, rows3, len, stride2) ||
            0 == endian_write_matrix(id, rows4, len, stride2)) {
          if ( 0 != errno) {
            perror(name);
          }
          fprintf(stderr, "%s: cannot write %d rows to one of %s or %s, terminating\n",
                  name, stride2, name_echelised, name_id);
          header_free(h);
          cleanup_all(echelised, name_echelised, id, name_id);
          exit(1);
        }
        ptr_e1 = ftello64(echelised);
        ptr_i1 = ftello64(id);
      }
      /* Now forward clean the later stuff */
      ptr_e1 = ptr_echelised; /* where we are in echelised */
      ptr_i1 = ptr_id; /* where we are in id */
      for (n = r + stride; n < nor; n += work_rows) {
        unsigned int stride2 = (work_rows + n > nor) ? nor - n : work_rows;
        /* Read stride2 rows from echelised and id at offsets ptr_e1, ptr_i1 */
        fseeko64(echelised, ptr_e1, SEEK_SET);
        fseeko64(id, ptr_i1, SEEK_SET);
        errno = 0;
        if (0 == endian_read_matrix(echelised, rows3, len, stride2) ||
            0 == endian_read_matrix(id, rows4, len, stride2)) {
          if ( 0 != errno) {
            perror(name);
          }
          fprintf(stderr, "%s: cannot read %d rows from one of %s or %s, terminating\n",
                  name, stride2, name_echelised, name_id);
          header_free(h);
          cleanup_all(echelised, name_echelised, id, name_id);
        }
        /* Clean the rows we read with the newly made rows */
        clean(rows1, stride, rows3, stride2, map1 + r, rows2, rows4, 1,
              grease.level, prime, len, nob, 900, 950, len, 0, name);
        /* Write back the cleaned version to echelised and id */
        fseeko64(echelised, ptr_e1, SEEK_SET);
        fseeko64(id, ptr_i1, SEEK_SET);
        errno = 0;
        if (0 == endian_write_matrix(echelised, rows3, len, stride2) ||
            0 == endian_write_matrix(id, rows4, len, stride2)) {
          if ( 0 != errno) {
            perror(name);
          }
          fprintf(stderr, "%s: cannot write %d rows to one of %s or %s, terminating\n",
                  name, stride2, name_echelised, name_id);
          header_free(h);
          cleanup_all(echelised, name_echelised, id, name_id);
          exit(1);
        }
        ptr_e1 = ftello64(echelised);
        ptr_i1 = ftello64(id);
      }
      /* Reset pointers to next chunk */
      fseeko64(echelised, ptr_echelised, SEEK_SET);
      fseeko64(id, ptr_id, SEEK_SET);
    }
    if (0 == open_and_write_binary_header(&outp, h, m2, name)) {
      exit(1);
    }
    header_free(h);
    /* Now write out the result rows in the right order */
    for (r = 0; r < nor; r += max_rows) {
      /* Interested in stride rows */
      unsigned int stride = (r + max_rows <= nor) ? max_rows : nor - r;
      /* Back to start in id file */
      fseeko64(id, 0, SEEK_SET);
      for (n = 0; n < nor; n++) {
        errno = 0;
        if (0 == endian_read_row(id, rows2[0], len)) {
          if ( 0 != errno) {
            perror(name);
          }
          fprintf(stderr, "%s cannot read row %d from %s, terminating\n", name, n, name_id);
          cleanup_all(echelised, name_echelised, id, name_id);
          fclose(outp);
          exit(1);
        }
        if (map1[n] >= (int)r && map1[n] < (int)(r + stride)) {
          /* We want this row at map1[n] - r */
          unsigned int offset = map1[n] - r;
          /* Swap the pointers to retain this row */
          unsigned int *row = rows2[0];
          rows2[0] = rows1[offset];
          rows1[offset] = row;
        }
      }
      for (n = 0; n < stride; n++) {
        errno = 0;
        if (0 == endian_write_row(outp, rows1[n], len)) {
          if ( 0 != errno) {
            perror(name);
          }
          fprintf(stderr, "%s: cannot write row %d to %s, terminating\n", name, r + n, m2);
          cleanup_all(echelised, name_echelised, id, name_id);
          fclose(outp);
          exit(1);
        }
      }
    }
    matrix_free(rows1);
    matrix_free(rows2);
    matrix_free(rows3);
    matrix_free(rows4);
    cleanup_all(echelised, name_echelised, id, name_id);
    free(map1);
    fclose(outp);
  }
}
