/*
 * $Id: maps.c,v 1.3 2002/06/28 08:39:16 jon Exp $
 *
 * Maps from {0 .. nor-1} -> {0 .. noc-1}
 *
 */

#include "maps.h"
#include <stdio.h>
#include <assert.h>
#include <errno.h>
#include "elements.h"
#include "endian.h"
#include "header.h"
#include "rows.h"
#include "utils.h"
#include "write.h"

unsigned int *malloc_map(unsigned int nor)
{
  return my_malloc(nor * sizeof(unsigned int));
}

void map_free(unsigned int *map)
{
  free(map);
}

/* Read one element from a map and set up as a matrix row */
int read_map_element_as_row(FILE *inp, unsigned int *row, unsigned int nob,
                            unsigned int noc, unsigned int len, const char *in, const char *name)
{
  unsigned int i;
  assert(NULL != inp);
  assert(NULL != row);
  assert(NULL != in);
  assert(NULL != name);
  errno = 0;
  if (1 != endian_read_int(&i, inp)) {
    if ( 0 != errno) {
      perror(name);
    }
    fprintf(stderr, "%s: failed to read entry from %s, terminating\n", name, in);
    return 0;
  }
  if (i >= noc) {
    fprintf(stderr, "%s: element %d from %s out of range (0 - %d), terminating\n", name, i, in, noc - 1);
    return 0;
  }
  row_init(row, len);
  put_element_to_row(nob, i, row, 1);
  return 1;
}

/* Assumes file pointing after header */
int read_map(FILE *inp, unsigned int nor, unsigned int *map, const char *name, const char *in)
{
  unsigned int i, j;
  assert(NULL != inp);
  assert(NULL != map);
  assert(NULL != name);
  assert(NULL != in);
  for (i = 0; i < nor; i++) {
    errno = 0;
    if (1 != endian_read_int(&j, inp)) {
      if ( 0 != errno) {
        perror(name);
      }
      fprintf(stderr, "%s: failed to read entry %d from %s, terminating\n", name, i, in);
      fclose(inp);
      return 0;
    }
    map[i] = j;
  }
  return 1;
}

int write_map(FILE *outp, unsigned int nor, unsigned int *map, const char *name, const char *out)
{
  unsigned int i;
  assert(NULL != outp);
  assert(NULL != map);
  assert(NULL != name);
  assert(NULL != out);
  for (i = 0; i < nor; i++) {
    errno = 0;
    if (1 != endian_write_int(map[i], outp)) {
      if ( 0 != errno) {
        perror(name);
      }
      fprintf(stderr, "%s: failed to write entry %d to %s, or entry out of range, terminating\n", name, i, out);
      fclose(outp);
      return 0;
    }
  }
  return 1;
}

int mul_map(unsigned int *in1, unsigned int *in2, unsigned int *out,
            const header *h1, const header *h2, const char *name)
{
  unsigned int i, j, k, nor1, nor2, noc1, noc2;
  assert(NULL != in1);
  assert(NULL != in2);
  assert(NULL != out);
  assert(NULL != h1);
  assert(NULL != h2);
  assert(NULL != name);
  assert(1 == header_get_prime(h1));
  assert(1 == header_get_prime(h2));
  nor1 = header_get_nor(h1);
  nor2 = header_get_nor(h2);
  noc1 = header_get_noc(h1);
  noc2 = header_get_noc(h2);
  if (noc1 != nor2) {
    fprintf(stderr, "%s: incompatible map multiplication requested, terminating\n", name);
    return 0;
  }
  for (i = 0; i < nor1; i++) {
    j = in1[i];
    if (j >= noc1) {
      fprintf(stderr, "%s: map entry %d out of range (0 - %d), terminating\n", name, j, noc1 - 1);
      return 0;
    }
    k = in2[j];
    if (k >= noc2) {
      fprintf(stderr, "%s: map entry %d out of range (0 - %d), terminating\n", name, k, noc2 - 1);
      return 0;
    }
    out[i] = k;
  }
  return 1;
}

int map_rank(FILE *inp, const header *h, const char *m, unsigned int *r, const char *name)
{
  unsigned int noc, nor, i, rn, *map1, *map2;
  assert(NULL != r);
  assert(NULL != m);
  assert(NULL != name);
  nor = header_get_nor(h);
  noc = header_get_noc(h);
  assert(1 == header_get_prime(h));
  map1 = malloc_map(nor);
  map2 = malloc_map(noc);
  for (i = 0; i < noc; i++) {
    map2[i] = nor; /* Initialise out of range */
  }
  if (0 == read_map(inp, nor, map1, name, m)) {
    map_free(map1);
    map_free(map2);
    return 0;
  }
  rn = nor;
  for (i = 0; i < nor; i++) {
    unsigned int j = map1[i];
    if (j >= noc) {
      fprintf(stderr, "%s: map entry %d out of range (0 - %d), terminating\n", name, j, noc - 1);
      map_free(map1);
      map_free(map2);
      return 0;
    }
    if (map2[j] == nor) {
      map2[j] = i;
    } else {
      rn--; /* Decrement one for each duplicated image */
    }
  }
  *r = rn;
  return 1;
}

int map_iv(FILE *inp, const header *h, const char *m1, const char *m2, const char *name)
{
  unsigned int noc, nor, i, j, *map1, *map2;
  FILE *outp;
  assert(NULL != inp);
  assert(NULL != h);
  assert(NULL != m1);
  assert(NULL != m2);
  assert(NULL != name);
  assert(1 == header_get_prime(h));
  noc = header_get_noc(h);
  nor = header_get_nor(h);
  if (nor != noc) {
    fprintf(stderr, "%s: cannot invert non-square map %s, terminating\n", name, m1);
    return 0;
  }
  map1 = malloc_map(nor);
  if (0 == read_map(inp, nor, map1, name, m1)) {
    map_free(map1);
    return 0;
  }
  map2 = malloc_map(nor);
  for (i = 0; i < nor; i++) {
    map2[i] = nor;
  }
  for (i = 0; i < nor; i++) {
    j = map1[i];
    if (j >= nor) {
      fprintf(stderr, "%s: map entry %d out of range (0 - %d), terminating\n", name, j, nor - 1);
      map_free(map1);
      map_free(map2);
      return 0;
    }
    if (map2[j] != nor) {
      fprintf(stderr, "%s: map %s is singular, terminating\n", name, m1);
      map_free(map1);
      map_free(map2);
      return 0;
    }
    map2[j] = i;
  }
  map_free(map1);
  if (0 == open_and_write_binary_header(&outp, h, m2, name)) {
    map_free(map1);
    map_free(map2);
    return 0;
  }
  if (0 == write_map(outp, nor, map2, name, m2)) {
    map_free(map1);
    map_free(map2);
    return 0;
  }
  map_free(map2);
  return 1;
}
