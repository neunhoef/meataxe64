/*
 * $Id: zrn.c,v 1.1 2001/11/12 13:43:38 jon Exp $
 *
 * Compute the rank of a matrix
 *
 */

#include <stdio.h>
#include "endian.h"
#include "memory.h"
#include "rn.h"

static const char *name = "zrn";

static void rn_usage(void)
{
  fprintf(stderr, "%s: usage: %s <in_file> [<memory>]\n", name, name);
}

int main(int argc, const char * const argv[])
{
  unsigned int n;
  unsigned int memory = MEM_SIZE;

  if (2 != argc && 3 != argc) {
    rn_usage();
    exit(1);
  }
  endian_init();
  if (3 == argc) {
    memory = strtoul(argv[2], NULL, 0);
  }
  memory_init(name, memory);
  n = rank(argv[1], name);
  printf("%d\n", n);
  memory_dispose();
  return 0;
}
