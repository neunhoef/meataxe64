/*
 * $Id: ss_map.c,v 1.2 2002/06/28 08:39:16 jon Exp $
 *
 * Function to compute subspace map
 *
 */

#include "ss_map.h"
#include <stdio.h>
#include <assert.h>
#include <errno.h>
#include "endian.h"
#include "elements.h"
#include "utils.h"

int subspace_map(FILE *inp, int *map, unsigned int nor,
                 unsigned int len, unsigned int nob,
                 unsigned int *row, const char *in,
                 const char *name)
{
  unsigned int i;
  assert(NULL != inp);
  assert(NULL != in);
  assert(NULL != map);
  assert(NULL != row);
  assert(NULL != name);
  assert(0 != nor);
  assert(0 != nob);
  assert(0 != len);
  /* Step through range to set up map */
  for (i = 0; i < nor; i += 1) {
    map[i] = -1;
  }
  for (i = 0; i < nor; i += 1) {
    unsigned int j, elt;
    errno = 0;
    if (0 == endian_read_row(inp, row, len)) {
      if ( 0 != errno) {
        perror(name);
      }
      fprintf(stderr, "%s: cannot read row from %s, terminating\n", name, in);
      return 0;
    }
    elt = first_non_zero(row, nob, len, &j);
    if (0 == elt || map[i] >= 0) {
      fprintf(stderr, "%s: %s is not linearly independent/echelised, terminating\n", name, in);
      return 0;
    }
    map[i] = j;
  }
  return 1;
}
