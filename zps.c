/*
 * $Id: zps.c,v 1.1 2002/06/25 10:30:12 jon Exp $
 *
 * Calculate permutation space representation
 *
 */

#include <stdio.h>
#include "endian.h"
#include "memory.h"
#include "ps.h"

static const char *name = "zps";

static void ps_usage(void)
{
  fprintf(stderr, "%s: usage: %s <range> <image> <output> [<memory>]\n", name, name);
}

int main(int argc, const char * const argv[])
{
  unsigned int memory = MEM_SIZE;

  if (4 != argc && 5 != argc) {
    ps_usage();
    exit(1);
  }
  endian_init();
  if (5 == argc) {
    memory = strtoul(argv[4], NULL, 0);
  }
  memory_init(name, memory);
  permutation_space(argv[1], argv[2], argv[3], name);
  memory_dispose();
  return 0;
}
