/*
 * $Id: join.c,v 1.2 2002/04/10 23:33:27 jon Exp $
 *
 * Function to append two matrices to give a third
 *
 */

#include "join.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "header.h"
#include "endian.h"
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
  if (0 == open_and_read_binary_header(&inp1, &h1, m1, name) ||
      0 == open_and_read_binary_header(&inp2, &h2, m2, name)) {
    if (NULL != inp1) {
      fclose(inp1);
      header_free(h1);
    }
    return 0;
  }
  prime = header_get_prime(h1);
  if (1 == prime) {
    fprintf(stderr, "%s: cannot handle maps, terminating\n", name);
    header_free(h1);
    header_free(h2);
    fclose(inp1);
    fclose(inp2);
    return 0;
  }
  nob = header_get_nob(h1);
  nod = header_get_nod(h1);
  nor1 = header_get_nor(h1);
  noc = header_get_noc(h1);
  len = header_get_len(h1);
  if (memory_rows(len, 1000) < 1) {
    fprintf(stderr, "%s cannot allocate rows for %s, %s, %s, terminating\n", name, m1, m2, m3);
    header_free(h1);
    header_free(h2);
    fclose(inp1);
    fclose(inp2);
    return 0;
  }
  if (header_get_prime(h2) != prime ||
      header_get_nob(h2) != nob ||
      header_get_noc(h2) != noc) {
    fprintf(stderr, "%s header mismatch between %s and %s, terminating\n", name, m1, m2);
    header_free(h1);
    header_free(h2);
    fclose(inp1);
    fclose(inp2);
    return 0;
  }
  nor2 = header_get_nor(h2);
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
    if (0 == endian_read_row(inp1, row, len)) {
      fprintf(stderr, "%s cannot read row %d from %s, terminating\n", name, i, m1);
      fclose(inp1);
      fclose(inp2);
      fclose(outp);
      return 0;
    }
    if (0 == endian_write_row(outp, row, len)) {
      fprintf(stderr, "%s cannot write row %d to %s, terminating\n", name, i, m3);
      fclose(inp1);
      fclose(inp2);
      fclose(outp);
      return 0;
    }
  }
  fclose(inp1);
  for (i = 0; i < nor2; i++) {
    if (0 == endian_read_row(inp2, row, len)) {
      fprintf(stderr, "%s cannot read row %d from %s, terminating\n", name, i, m2);
      fclose(inp2);
      fclose(outp);
      return 0;
    }
    if (0 == endian_write_row(outp, row, len)) {
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
