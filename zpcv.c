/*
 * $Id: zpcv.c,v 1.3 2002/10/14 19:11:51 jon Exp $
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
  fprintf(stderr, "%s: usage: %s [-v] [-m <memory>] <orbit set> <vectors> <output>\n", name, name);
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
