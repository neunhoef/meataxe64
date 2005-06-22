/*
 * $Id: clean_file.c,v 1.6 2005/06/22 21:52:53 jon Exp $
 *
 * Cleaning and echilisation, when the already clean vectors
 * are in a file which is to be updated
 *
 */

#include "clean_file.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <errno.h>
#include "clean.h"
#include "endian.h"
#include "utils.h"

/* Clean input rows1 with clean_vectors, and extend clean_vectors */
/* Return the new number of clean vectors */
/* Record the map extension */
/* nor is total rows in file, nor1 is number in rows1 */
/* nor2 is space available in rows2 (scratch) */
/* Also, if asked, return the map for input rows (needed for standard base) */
int clean_file(row_ops *row_operations,
               FILE *clean_vectors, u32 *nor,
               word **rows1, u32 nor1,
               word **rows2, u32 nor2,
               int *map, int *new_map, int record,
               u32 grease_level, u32 prime,
               u32 len, u32 nob,
               u32 start, const char *name)
{ 
  u32 i, j = 0, my_nor, d;
  int *internal_new_map;
  long long ptr;
  assert(NULL != clean_vectors);
  assert(NULL != rows1);
  assert(NULL != rows2);
  assert(NULL != map);
  assert(NULL != name);
  assert(NULL != nor);
  assert(0 != prime);
  assert(0 != len);
  assert(0 != nob);
  assert(0 != nor1);
  if (0 != record) {
    assert(NULL != new_map);
  }
  my_nor = *nor;
  if (0 != my_nor) {
    fseeko64(clean_vectors, 0, SEEK_SET);
    for (i = 0; i < my_nor; i += nor2) {
      u32 stride = (i + nor2 > my_nor) ? my_nor - i : nor2;
      errno = 0;
      if (0 == endian_read_matrix(clean_vectors, rows2, len, stride)) {
        if ( 0 != errno) {
          perror(name);
        }
        fprintf(stderr, "%s: failed to read %d rows from temporary file, terminating\n", name, stride);
        return 0;
      }
      clean(row_operations, rows2, stride, rows1, nor1, map + i, NULL, NULL, 0,
            grease_level, prime, len, nob, start, 0, 0, 0, name);
    }
  }
  echelise(row_operations, rows1, nor1, &d, &internal_new_map, NULL, 0,
           grease_level, prime, len, nob, start, 0, 0, 1, name);
  if (0 != d) {
    if (0 != my_nor) {
      ptr = 0; /* where we are in echelised */
      for (i = 0; i < my_nor; i += nor2) {
        u32 stride2 = (nor2 + i > my_nor) ? my_nor - i : nor2;
        /* Read stride2 rows from echelised at offset ptr */
        fseeko64(clean_vectors, ptr, SEEK_SET);
        errno = 0;
        if (0 == endian_read_matrix(clean_vectors, rows2, len, stride2)) {
          if ( 0 != errno) {
            perror(name);
          }
          fprintf(stderr, "%s: cannot read temporary matrix, terminating\n", name);
          return 0;
        }
        /* Clean the rows we read with the newly made rows */
        clean(row_operations, rows1, nor1, rows2, stride2, internal_new_map, NULL, NULL, 0,
              grease_level, prime, len, nob, start, 0, 0, 0, name);
        /* Write back the cleaned version to echelised */
        fseeko64(clean_vectors, ptr, SEEK_SET);
        errno = 0;
        if (0 == endian_write_matrix(clean_vectors, rows2, len, stride2)) {
          if ( 0 != errno) {
            perror(name);
          }
          fprintf(stderr, "%s: cannot write temporary matrix, terminating\n", name);
          return 0;
        }
        ptr = ftello64(clean_vectors);
      }
    }
    /* We put the new rows onto the end */
    fseeko64(clean_vectors, 0, SEEK_END);
    for (i = 0; i < nor1; i++) {
      if (internal_new_map[i] >= 0) {
        /* Got a useful row */
        map[my_nor + j] = internal_new_map[i];
        errno = 0;
        if (0 == endian_write_row(clean_vectors, rows1[i], len)) {
          if ( 0 != errno) {
            perror(name);
          }
          fprintf(stderr, "%s: failed to write to temporary matrix, terminating\n",
                  name);
          return 0;
        }
        j++;
      }
    }
  }
  if (0 != record) {
    /* Remember the map for the caller */
    memcpy(new_map, internal_new_map, nor1 * sizeof(int));
  } else {
    NOT_USED(new_map);
  }
  free(internal_new_map);
  assert(j == d);
  my_nor += d; /* The number of extra rows we made */
  *nor = my_nor;
  return 1;
}
