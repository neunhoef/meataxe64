/*
 * $Id: zpcv.c,v 1.1 2002/06/25 10:30:12 jon Exp $
 *
 * Calculate lift from permutation condensation representation
 *
 */

#include <stdio.h>
#include "endian.h"
#include "memory.h"
#include "pcv.h"

static const char *name = "zpcv";

static void pcv_usage(void)
{
  fprintf(stderr, "%s: usage: %s <orbit set> <vectors> <output> [<memory>]\n", name, name);
}

int main(int argc, const char * const argv[])
{
  unsigned int memory = MEM_SIZE;

  if (4 != argc && 5 != argc) {
    pcv_usage();
    exit(1);
  }
  endian_init();
  if (5 == argc) {
    memory = strtoul(argv[4], NULL, 0);
  }
  memory_init(name, memory);
  lift(argv[1], argv[2], argv[3], name);
  memory_dispose();
  return 0;
}
