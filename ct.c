/*
 * $Id: ct.c,v 1.2 2001/10/16 22:55:53 jon Exp $
 *
 * Count the non-zero elements in a matrix
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include "count.h"

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
  total = count(argv[1], name);
  printf("%u\n", total);
  return 0;
}
