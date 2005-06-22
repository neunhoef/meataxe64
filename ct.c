/*
 * $Id: ct.c,v 1.7 2005/06/22 21:52:53 jon Exp $
 *
 * Count the non-zero elements in a matrix
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include "count.h"
#include "endian.h"
#include "memory.h"
#include "parse.h"

static const char *name = "zct";

static void ct_usage(void)
{
  fprintf(stderr, "%s: usage: %s %s <in_file>\n", name, name, parse_usage());
}

int main(int argc, const char * const argv[])
{
  u32 total;
  argv = parse_line(argc, argv, &argc);
  if (2 != argc) {
    ct_usage();
    exit(1);
  }
  endian_init();
  memory_init(name, memory);
  total = count(argv[1], name);
  printf("%u\n", total);
  return 0;
}
