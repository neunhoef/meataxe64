/*
 * $Id: map_or_row.c,v 1.4 2005/07/24 09:32:45 jon Exp $
 *
 * Handle reading from a choice of map or row
 *
 */

#include "map_or_row.h"
#include <stdio.h>
#include <assert.h>
#include <errno.h>
#include "endian.h"
#include "maps.h"

int read_row(int is_perm, FILE *inp, word *row,
             u32 nob, u32 noc, u32 len,
             u32 i, const char *m, const char *name)
{
  assert(NULL != inp);
  assert(NULL != row);
  assert(NULL != m);
  assert(NULL != name);
  if (is_perm) {
    if (0 == read_map_element_as_row(inp, row, nob, noc, len, m, name)) {
      return 0;
    }
  } else {
    errno = 0;
    if (0 == endian_read_row(inp, row, len)) {
      if ( 0 != errno) {
        perror(name);
      }
      fprintf(stderr, "%s: cannot read row %u from %s, terminating\n", name, i, m);
      return 0;
    }
  }
  return 1;
}

int read_rows(int is_perm, FILE *inp, word **rows,
              u32 nob, u32 noc, u32 len,
              u32 nor, const char *m, const char *name)
{
  u32 i;
  for (i = 0; i < nor; i++) {
    if (0 == read_row(is_perm, inp, rows[i], nob, noc, len, i, m, name)) {
      return 0;
    }
  }
  return 1;
}
