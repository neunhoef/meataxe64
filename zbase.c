/*
 * $Id: zbase.c,v 1.2 2002/02/05 19:50:56 jon Exp $
 *
 * Compute a basis for a space from a set of vectors
 *
 */

#include <stdio.h>
#include "endian.h"
#include "memory.h"
#include "rnf.h"

static const char *name = "zbase";

static void base_usage(void)
{
  fprintf(stderr, "%s: usage: %s <in_file> <temp dir> <output> [<memory>]\n", name, name);
}

int main(int argc, const char * const argv[])
{
  unsigned int n;
  unsigned int memory = MEM_SIZE;

  if (4 != argc && 5 != argc) {
    base_usage();
    exit(1);
  }
  endian_init();
  if (5 == argc) {
    memory = strtoul(argv[4], NULL, 0);
  }
  memory_init(name, memory);
  n = rank(argv[1], argv[2], argv[3], 1, name);
  printf("%d\n", n);
  memory_dispose();
  return 0;
}
