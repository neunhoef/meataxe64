/*
 * $Id: map_or_row.c,v 1.1 2002/06/25 10:30:12 jon Exp $
 *
 * Handle reading from a choice of map or row
 *
 */

#include "map_or_row.h"
#include "maps.h"
#include <stdio.h>
#include <assert.h>
#include "endian.h"

int read_row(int is_perm, FILE *inp, unsigned int *row,
             unsigned int nob, unsigned int noc, unsigned int len,
             unsigned int i, const char *m, const char *name)
{
  assert(NULL != inp);
  assert(NULL != row);
  assert(NULL != m);
  assert(NULL != name);
  if (is_perm) {
    if (0 == read_map_element_as_row(inp, row, nob, noc, len, m, name)) {
      fprintf(stderr, "%s: cannot read row %d from %s, terminating\n", name, i, m);
      return 0;
    }
  } else {
    if (0 == endian_read_row(inp, row, len)) {
      fprintf(stderr, "%s: cannot read row %d from %s, terminating\n", name, i, m);
      return 0;
    }
  }
  return 1;
}

int read_rows(int is_perm, FILE *inp, unsigned int **rows,
              unsigned int nob, unsigned int noc, unsigned int len,
              unsigned int nor, const char *m, const char *name)
{
  unsigned int i;
  for (i = 0; i < nor; i++) {
    if (0 == read_row(is_perm, inp, rows[i], nob, noc, len, i, m, name)) {
      return 0;
    }
  }
  return 1;
}
