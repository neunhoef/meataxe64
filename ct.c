/*
 * $Id: ct.c,v 1.4 2002/07/09 09:08:12 jon Exp $
 *
 * Count the non-zero elements in a matrix
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include "count.h"
#include "endian.h"
#include "parse.h"

static const char *name = "zct";

static void ct_usage(void)
{
  fprintf(stderr, "%s: usage: %s <in_file>\n", name, name);
}

int main(int argc, const char * const argv[])
{
  unsigned int total;
  argv = parse_line(argc, argv, &argc);
  if (2 != argc) {
    ct_usage();
    exit(1);
  }
  endian_init();
  total = count(argv[1], name);
  printf("%u\n", total);
  return 0;
}
