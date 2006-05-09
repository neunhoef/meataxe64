/*
 * $Id: pfl.c,v 1.1 2006/05/09 22:09:28 jon Exp $
 *
 * Print a filter list
 *
 */

#include "pfl.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#include "endian.h"
#include "elements.h"
#include "header.h"
#include "memory.h"
#include "read.h"
#include "utils.h"

void print_filter(const char *in_list,
                  unsigned int argc, const char * const args[],
                  const char *name)
{
  u32 nor, noc, nob, len, prime, i;
  FILE *inp;
  const header *h;
  word *row;
  NOT_USED(args);
  /* open input */
  if (0 == open_and_read_binary_header(&inp, &h, in_list, name)) {
    exit(1);
  }
  /* check argc against noc */
  nor = header_get_nor(h);
  noc = header_get_noc(h);
  nob = header_get_nob(h);
  prime = header_get_prime(h);
  len = header_get_len(h);
  header_free(h);
  if (noc != argc) {
    fprintf(stderr, "%s: columns and args don't match, terminating\n", name);
    fclose(inp);
    exit(1);
  }
  if (memory_rows(len, 1000) < 1) {
    fprintf(stderr, "%s: cannot allocate 1 row, terminating\n", name);
    fclose(inp);
    exit(1);
  }
  row = memory_pointer(0);
  while (nor > 0) {
    /* read a row */
    errno = 0;
    if (0 == endian_read_row(inp, row, len)) {
      if ( 0 != errno) {
        perror(name);
      }
      fprintf(stderr, "%s: cannot read row from %s, terminating\n", name, in_list);
      fclose(inp);
      exit(1);
    }
    /* print sum of generators */
    printf("I");
    for (i = 0; i < noc; i++) {
      word elt = get_element_from_row(nob, i, row);
      if (0 !=elt) {
        printf("+%" W_F "%s", elt, args[i]);
      }
    }
    printf("\n");
    nor--;
  }
  fclose(inp);
}
