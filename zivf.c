/*
 * $Id: zivf.c,v 1.1 2002/04/15 07:47:23 jon Exp $
 *
 * Invert a matrix
 *
 */

#include <stdio.h>
#include "endian.h"
#include "ivf.h"
#include "memory.h"

static const char *name = "zivf";

static void ivf_usage(void)
{
  fprintf(stderr, "%s: usage: %s <in_file> <out_file> <temp dir> [<memory>]\n", name, name);
}

int main(int argc, const char * const argv[])
{
  unsigned int memory = MEM_SIZE;

  if (4 != argc && 5 != argc) {
    ivf_usage();
    exit(1);
  }
  endian_init();
  if (5 == argc) {
    memory = strtoul(argv[4], NULL, 0);
  }
  memory_init(name, memory);
  invert(argv[1], argv[2], argv[3], name);
  memory_dispose();
  return 0;
}
