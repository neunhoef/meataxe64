/*
 * $Id: zpcv.c,v 1.4 2004/01/04 21:22:50 jon Exp $
 *
 * Calculate lift from permutation condensation representation
 *
 */

#include <stdio.h>
#include "endian.h"
#include "memory.h"
#include "parse.h"
#include "pcv.h"

static const char *name = "zpcv";

static void pcv_usage(void)
{
  fprintf(stderr, "%s: usage: %s %s <orbit set> <vectors> <output>\n", name, name, parse_usage());
}

int main(int argc, const char * const argv[])
{
  argv = parse_line(argc, argv, &argc);
  if (4 != argc) {
    pcv_usage();
    exit(1);
  }
  endian_init();
  memory_init(name, memory);
  lift(argv[1], argv[2], argv[3], name);
  memory_dispose();
  return 0;
}
