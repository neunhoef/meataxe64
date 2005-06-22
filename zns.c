/*
 * $Id: zns.c,v 1.7 2005/06/22 21:52:54 jon Exp $
 *
 * Compute the null space of a matrix
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include "endian.h"
#include "memory.h"
#include "ns.h"
#include "parse.h"

static const char *name = "zns";

static void ns_usage(void)
{
  fprintf(stderr, "%s: usage: %s %s <in_file> <out_file>\n", name, name, parse_usage());
}

int main(int argc, const char * const argv[])
{
  u32 n;

  argv = parse_line(argc, argv, &argc);
  if (3 != argc) {
    ns_usage();
    exit(1);
  }
  endian_init();
  memory_init(name, memory);
  n = nullspace(argv[1], argv[2], name);
  printf("%d\n", n);
  memory_dispose();
  return 0;
}
