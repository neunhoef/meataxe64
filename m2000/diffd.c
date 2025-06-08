/*
 * $Id: diffd.c,v 1.5 2005/07/24 11:31:35 jon Exp $
 *
 * Function to find the differences between the diagonal of a matrix and a scalar
 *
 */

#include "diffd.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#include "elements.h"
#include "endian.h"
#include "header.h"
#include "memory.h"
#include "read.h"
#include "utils.h"

int diffd(const char *m, u32 elt, const char *name)
{
  FILE *inp = NULL;
  u32 prime, nob, nor, len;
  u32 i;
  const header *h;
  word *row;

  assert(NULL != m);
  assert(NULL != name);
  if (0 == open_and_read_binary_header(&inp, &h, m, name)) {
    exit(1);
  }
  prime = header_get_prime(h);
  nob = header_get_nob(h);
  nor = header_get_nor(h);
  len = header_get_len(h);
  if (1 == prime) {
    fprintf(stderr, "%s: cannot compare map diagonal with field element, terminating\n", name);
    fclose(inp);
    header_free(h);
    exit(1);
  }
  header_free(h);
  if (memory_rows(len, 1000) < 1) {
    fprintf(stderr, "%s cannot allocate row for %s, terminating\n", name, m);
    fclose(inp);
    exit(2);
  }
  row = memory_pointer_offset(0, 0, len);
  for (i = 0; i < nor; i++) {
    errno = 0;
    if (0 == endian_read_row(inp, row, len)) {
      if ( 0 != errno) {
        perror(name);
      }
      fprintf(stderr, "%s cannot read row %u from %s, terminating\n", name, i, m);
      fclose(inp);
      exit(1);
    }
    if (get_element_from_row(nob, i, row) != elt) {
      fclose(inp);
      return 0;
    }
  }
  fclose(inp);
  return 1;
}
