/*
 * $Id: zrn.c,v 1.4 2002/10/13 20:07:20 jon Exp $
 *
 * Compute the rank of a matrix
 *
 */

#include <stdio.h>
#include "endian.h"
#include "memory.h"
#include "parse.h"
#include "rn.h"

static const char *name = "zrn";

static void rn_usage(void)
{
  fprintf(stderr, "%s: usage: %s [-v] [-m <memory>] <in_file>\n", name, name);
}

int main(int argc, const char * const argv[])
{
  unsigned int n;

  argv = parse_line(argc, argv, &argc);
  if (2 != argc) {
    rn_usage();
    exit(1);
  }
  endian_init();
  memory_init(name, memory);
  if (0 == rank(argv[1], &n, name)) {
    exit(1);
  }
  printf("%d\n", n);
  memory_dispose();
  return 0;
}
