/*
 * $Id: ps.c,v 1.3 2002/06/28 08:39:16 jon Exp $
 *
 * Function to compute permutation space representation
 *
 */

#include "ps.h"
#include "endian.h"
#include "header.h"
#include "memory.h"
#include "matrix.h"
#include "read.h"
#include "utils.h"
#include "write.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

typedef struct vec_struct
{
  unsigned int index;
  unsigned int hash;
  unsigned int *row;
} vec;

static unsigned int row_len = 0;

static int compar(const void *e1, const void *e2)
{
  vec **v1 = (vec **)e1;
  vec **v2 = (vec **)e2;
  unsigned int h1 = (*v1)->hash;
  unsigned int h2 = (*v2)->hash;
  if (h1 != h2) {
    return (h1 < h2) ? -1 : 1;
  }
  /* Hashes equal */
  return memcmp((*v1)->row, (*v2)->row, row_len * sizeof(unsigned int));
}

static void cleanup(FILE *f1, FILE *f2, FILE *f3)
{
  if (NULL != f1)
    fclose(f1);
  if (NULL != f2)
    fclose(f2);
  if (NULL != f3)
    fclose(f3);
}

static unsigned int hash_fn(unsigned int *row, unsigned int len)
{
  unsigned int res = 0, i;
  assert(NULL != row);
  for (i = 0; i < len; i++) {
    res ^= row[i];
  }
  return res;
}

void permutation_space(const char *range, const char *image,
                       const char *out, const char *name)
{
  FILE *inp1 = NULL, *inp2 = NULL, *outp = NULL;
  const header *h_in1, *h_in2, *h_out;
  unsigned int prime, nob, noc, nor, nor_o, noc_o, len, i, hash_len;
  unsigned int *row_in, **rows;
  unsigned int *hashes;
  vec *records, **record_ptrs;
  assert(NULL != range);
  assert(NULL != image);
  assert(NULL != out);
  assert(NULL != name);
  if (0 == open_and_read_binary_header(&inp1, &h_in1, range, name) ||
      0 == open_and_read_binary_header(&inp2, &h_in2, image, name)) {
    cleanup(inp1, inp2, NULL);
    exit(1);
  }
  prime = header_get_prime(h_in1);
  if (1 == prime) {
    fprintf(stderr, "%s: cannot handle maps, terminating\n", name);
    cleanup(inp1, inp2, NULL);
    exit(1);
  }
  nob = header_get_nob(h_in1);
  nor = header_get_nor(h_in1);
  noc = header_get_noc(h_in1);
  len = header_get_len(h_in1);
  row_len = len;
  noc_o = nor;
  nor_o = nor; /* Output will be square */
  if (noc != header_get_noc(h_in2) ||
      prime != header_get_prime(h_in2) ||
      nob != header_get_nob(h_in2)) {
    fprintf(stderr, "%s: incompatible parameters for %s, %s, terminating\n",
            name, range, image);
    cleanup(inp1, inp2, NULL);
    exit(1);
  }
  if (memory_rows(len, 1000) < nor + 1) {
    fprintf(stderr, "%s: cannot allocate space for %d rows from %s, terminating\n",
            name, nor + 1, range);
    cleanup(inp1, inp2, NULL);
    exit(1);
  }
  hash_len = len / 10;
  if (0 == hash_len) {
    hash_len = 1;
  } else if (10 < hash_len) {
    hash_len = 10;
  }
  rows = matrix_malloc(nor);
  records = my_malloc(nor * sizeof(vec));
  record_ptrs = my_malloc(nor * sizeof(vec *));
  for (i = 0; i < nor; i++) {
    rows[i] = memory_pointer_offset(0, i, len);
    record_ptrs[i] = records + i;
  }
  row_in = memory_pointer_offset(0, nor, len);
  h_out = header_create(1, 0, 0, noc_o, nor_o);
  assert(header_get_len(h_in2) == len);
  header_free(h_in1);
  header_free(h_in2);
  if (0 == open_and_write_binary_header(&outp, h_out, out, name)) {
    cleanup(inp1, inp2, outp);
    exit(1);
  }
  header_free(h_out);
  errno = 0;
  if (0 == endian_read_matrix(inp1, rows, len, nor)) {
    if ( 0 != errno) {
      perror(name);
    }
    fprintf(stderr, "%s: unable to read %s, terminating\n", name, range);
    cleanup(inp1, inp2, outp);
    exit(1);
  }
  /* inp1 no longer necessary */
  fclose(inp1);
  hashes = my_malloc(nor_o * sizeof(unsigned int));
  for (i = 0; i < nor_o; i += 1) {
    hashes[i] = hash_fn(rows[i], hash_len);
    records[i].hash = hashes[i];
    records[i].index = i;
    records[i].row = rows[i];
  }
  /* Sort the pointers to the vectors */
  qsort(record_ptrs, nor, sizeof(vec *), &compar);
  /* Now step through image computing subspace representation */
  for (i = 0; i < nor_o; i += 1) {
    /* Read row of image */
    unsigned int row_out = 0, hash;
    vec row_vec, **found_row, *row_vec_ptr = &row_vec;
    errno = 0;
    if (0 == endian_read_row(inp2, row_in, len)) {
      if ( 0 != errno) {
        perror(name);
      }
      fprintf(stderr, "%s: cannot read row from %s, terminating\n", name, image);
      cleanup(inp1, inp2, outp);
      exit(1);
    }
    hash = hash_fn(row_in, hash_len);
    row_vec.hash = hash;
    row_vec.index = 0xffffffff;
    row_vec.row = row_in;
    /* Find the element in the range */
    found_row = bsearch(&row_vec_ptr, record_ptrs, nor, sizeof(vec *), &compar);
    if (NULL != found_row) {
      row_out = (*found_row)->index;
    } else {
      fprintf(stderr, "%s: cannot find image %d from %s, terminating\n", name, i, image);
      cleanup(inp1, inp2, outp);
      exit(1);
    }
    errno = 0;
    if (0 == endian_write_int(row_out, outp)) {
      if ( 0 != errno) {
        perror(name);
      }
      fprintf(stderr, "%s: cannot write row to %s, terminating\n", name, image);
      cleanup(inp1, inp2, outp);
      exit(1);
    }
  }
  matrix_free(rows);
  free(record_ptrs);
  free(records);
  free(hashes);
  fclose(inp2);
  fclose(outp);
}
