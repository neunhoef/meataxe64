/*
 * $Id: zns.c,v 1.3 2002/10/13 20:07:20 jon Exp $
 *
 * Compute the null space of a matrix
 *
 */

#include <stdio.h>
#include "endian.h"
#include "memory.h"
#include "ns.h"
#include "parse.h"

static const char *name = "zns";

static void ns_usage(void)
{
  fprintf(stderr, "%s: usage: %s [-v] [-m <memory>] <in_file> <out_file> [<memory>]\n", name, name);
}

int main(int argc, const char * const argv[])
{
  unsigned int n;

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
