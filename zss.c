/*
 * $Id: zss.c,v 1.1 2001/11/25 00:17:19 jon Exp $
 *
 * Calculate subspace representation
 *
 */

#include <stdio.h>
#include "endian.h"
#include "memory.h"
#include "ss.h"

static const char *name = "zss";

static void ss_usage(void)
{
  fprintf(stderr, "%s: usage: %s <range> <image> <output> [<memory>]\n", name, name);
}

int main(int argc, const char * const argv[])
{
  unsigned int memory = MEM_SIZE;

  if (4 != argc && 5 != argc) {
    ss_usage();
    exit(1);
  }
  endian_init();
  if (5 == argc) {
    memory = strtoul(argv[4], NULL, 0);
  }
  memory_init(name, memory);
  subspace(argv[1], argv[2], argv[3], name);
  memory_dispose();
  return 0;
}
