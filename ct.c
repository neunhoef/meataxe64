/*
 * $Id: ct.c,v 1.3 2002/01/06 16:35:48 jon Exp $
 *
 * Count the non-zero elements in a matrix
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include "count.h"
#include "endian.h"

static const char *name = "zct";

static void ct_usage(void)
{
  fprintf(stderr, "%s: usage: %s <in_file>\n", name, name);
}

int main(int argc, const char * const argv[])
{
  unsigned int total;
  if (2 != argc) {
    ct_usage();
    exit(1);
  }
  endian_init();
  total = count(argv[1], name);
  printf("%u\n", total);
  return 0;
}
