/*
 * $Id: join.c,v 1.4 2002/06/28 08:39:16 jon Exp $
 *
 * Function to append two matrices to give a third
 *
 */

#include "join.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#include "header.h"
#include "endian.h"
#include "map_or_row.h"
#include "memory.h"
#include "read.h"
#include "write.h"

int join(const char *m1, const char *m2, const char *m3, const char *name)
{
  FILE *inp1 = NULL;
  FILE *inp2 = NULL;
  FILE *outp = NULL;
  unsigned int prime, nob, nod, noc, nor1, nor2, len;
  unsigned int i;
  const header *h1, *h2;
  header *h;
  unsigned int *row;
  int is_perm1, is_perm2;
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
  if (is_perm1 && is_perm2) {
    fprintf(stderr, "%s: cannot handle join of two maps, terminating\n", name);
    header_free(h1);
    header_free(h2);
    fclose(inp1);
    fclose(inp2);
    return 0;
  }
  nob = (is_perm2) ? header_get_nob(h1) : header_get_nob(h2);
  len = (is_perm2) ? header_get_len(h1) : header_get_len(h2);
  nod = (is_perm2) ? header_get_nod(h1) : header_get_nod(h2);
  nor1 = header_get_nor(h1);
  noc = header_get_noc(h1);
  if (memory_rows(len, 1000) < 1) {
    fprintf(stderr, "%s cannot allocate rows for %s, %s, %s, terminating\n", name, m1, m2, m3);
    header_free(h1);
    header_free(h2);
    fclose(inp1);
    fclose(inp2);
    return 0;
  }
  if ((0 == is_perm1 && 0 == is_perm2 &&
       (header_get_prime(h2) != prime ||
        header_get_nob(h2) != nob)) ||
      header_get_noc(h2) != noc) {
    fprintf(stderr, "%s header mismatch between %s and %s, terminating\n", name, m1, m2);
    header_free(h1);
    header_free(h2);
    fclose(inp1);
    fclose(inp2);
    return 0;
  }
  nor2 = header_get_nor(h2);
  if (is_perm1) {
    prime = header_get_prime(h2);
  }
  header_free(h1);
  header_free(h2);
  h = header_create(prime, nob, nod, noc, nor1 + nor2);
  if (0 == open_and_write_binary_header(&outp, h1, m3, name)) {
    fclose(inp1);
    fclose(inp2);
    return 0;
  }
  row = memory_pointer_offset(0, 0, len);
  for (i = 0; i < nor1; i++) {
    if (0 == read_row(is_perm1, inp1, row, nob, noc, len, i, m1, name)) {
      fclose(inp1);
      fclose(inp2);
      fclose(outp);
      return 0;
    }
    errno = 0;
    if (0 == endian_write_row(outp, row, len)) {
      if ( 0 != errno) {
        perror(name);
      }
      fprintf(stderr, "%s cannot write row %d to %s, terminating\n", name, i, m3);
      fclose(inp1);
      fclose(inp2);
      fclose(outp);
      return 0;
    }
  }
  fclose(inp1);
  for (i = 0; i < nor2; i++) {
    if (0 == read_row(is_perm2, inp2, row, nob, noc, len, i, m2, name)) {
      fclose(inp2);
      fclose(outp);
      return 0;
    }
    errno = 0;
    if (0 == endian_write_row(outp, row, len)) {
      if ( 0 != errno) {
        perror(name);
      }
      fprintf(stderr, "%s cannot write row %d to %s, terminating\n", name, i, m3);
      fclose(inp2);
      fclose(outp);
      return 0;
    }
  }
  fclose(inp2);
  fclose(outp);
  return 1;
}
