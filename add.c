/*
 * $Id: add.c,v 1.21 2005/06/22 21:52:53 jon Exp $
 *
 * Function to add two matrices to give a third
 *
 */

#include "add.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <limits.h>
#include <errno.h>
#include "elements.h"
#include "endian.h"
#include "header.h"
#include "map_or_row.h"
#include "maps.h"
#include "memory.h"
#include "read.h"
#include "rows.h"
#include "utils.h"
#include "write.h"

static int cleanup(FILE *inp1, FILE *inp2, FILE *outp)
{
  if (NULL != inp1) {
    fclose(inp1);
  }
  if (NULL != inp2) {
    fclose(inp2);
  }
  if (NULL != outp) {
    fclose(outp);
  }
  return 0;
}

static int add_sub(const char *m1, const char *m2, const char *m3, const char *name, int scale, u32 elt)
{
  FILE *inp1 = NULL;
  FILE *inp2 = NULL;
  FILE *outp = NULL;
  u32 prime, nob, noc, nor, len;
  u32 i;
  int is_perm1, is_perm2;
  const header *h1, *h2, *h;
  word *row1, *row2;
  row_ops row_operations;
  row_incer incer;
  scaled_row_incer scaled_incer;

  if (0 == open_and_read_binary_header(&inp1, &h1, m1, name) ||
      0 == open_and_read_binary_header(&inp2, &h2, m2, name)) {
    if (NULL != inp1) {
      fclose(inp1);
      header_free(h1);
    }
    return 0;
  }
  prime = header_get_prime(h1);
  is_perm1 = 1 == prime;
  is_perm2 = 1 == header_get_prime(h2);
  nob = (is_perm2) ? header_get_nob(h1) : header_get_nob(h2);
  nor = header_get_nor(h1);
  noc = header_get_noc(h1);
  len = (is_perm2) ? header_get_len(h1) : header_get_len(h2);
  if (is_perm1 && is_perm2) {
    fprintf(stderr, "%s: cannot handle sum of two maps, terminating\n", name);
    header_free(h1);
    header_free(h2);
    return cleanup(inp1, inp2, NULL);
  }
  if ((header_get_prime(h2) != prime && (!is_perm1) && (!is_perm2)) ||
      (header_get_nob(h2) != nob && (!is_perm1) && (!is_perm2)) ||
      header_get_noc(h2) != noc ||
      header_get_nor(h2) != nor) {
    fprintf(stderr, "%s: header mismatch between %s and %s, terminating\n", name, m1, m2);
    header_free(h1);
    header_free(h2);
    return cleanup(inp1, inp2, NULL);
  }
  h = h1;
  if (1 == prime) {
    prime = header_get_prime(h2);
    h = h2;
  }
  elt = elt % prime; /* Bring into range */
  if (0 == rows_init(prime, &row_operations)) {
    fprintf(stderr, "%s: cannot initialise row operations for %s, %s, terminating\n", name, m1, m2);
    header_free(h1);
    header_free(h2);
    return cleanup(inp1, inp2, NULL);
  }
  incer = row_operations.incer;
  scaled_incer = row_operations.scaled_incer;
  if (0 == open_and_write_binary_header(&outp, h, m3, name)) {
    header_free(h1);
    header_free(h2);
    return cleanup(inp1, inp2, NULL);
  }
  header_free(h2);
  header_free(h1);
  if (memory_rows(len, 1000) < 2) {
    fprintf(stderr, "%s: cannot allocate rows for %s, %s, %s, terminating\n", name, m1, m2, m3);
    return cleanup(inp1, inp2, outp);
  }
  row1 = memory_pointer_offset(0, 0, len);
  row2 = memory_pointer_offset(0, 1, len);
  for (i = 0; i < nor; i++) {
    if (0 == read_row(is_perm1, inp1, row1, nob, noc, len, i, m1, name)) {
      return cleanup(inp1, inp2, outp);
    }
    if (0 == read_row(is_perm2, inp2, row2, nob, noc, len, i, m2, name)) {
      return cleanup(inp1, inp2, outp);
    }
    if (scale && 1 != elt) {
      if (0 != elt) {
        (*scaled_incer)(row1, row2, len, elt);
      }
    } else {
      (*incer)(row1, row2, len);
    }
    errno = 0;
    if (0 == endian_write_row(outp, row2, len)) {
      if ( 0 != errno) {
        perror(name);
      }
      fprintf(stderr, "%s: cannot write row %d to %s, terminating\n", name, i, m3);
      return cleanup(inp1, inp2, outp);
    }
  }
  (void)cleanup(inp1, inp2, outp);
  return 1;
}

int add(const char *m1, const char *m2, const char *m3, const char *name)
{
  return add_sub(m1, m2, m3, name, 0, 0);
}

int scaled_add(const char *m1, const char *m2, const char *m3, u32 scalar, const char *name)
{
  return add_sub(m1, m2, m3, name, 1, scalar);
}
