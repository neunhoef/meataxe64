/*
 * $Id: zrnf.c,v 1.1 2001/12/15 20:47:27 jon Exp $
 *
 * Compute the rank of a matrix, using temporary files
 *
 */

#include <stdio.h>
#include "endian.h"
#include "memory.h"
#include "rnf.h"

static const char *name = "zrnf";

static void rnf_usage(void)
{
  fprintf(stderr, "%s: usage: %s <in_file> <temp dir> [<memory>]\n", name, name);
}

int main(int argc, const char * const argv[])
{
  unsigned int n;
  unsigned int memory = MEM_SIZE;

  if (3 != argc && 4 != argc) {
    rnf_usage();
    exit(1);
  }
  endian_init();
  if (4 == argc) {
    memory = strtoul(argv[3], NULL, 0);
  }
  memory_init(name, memory);
  n = rank(argv[1], argv[2], name);
  printf("%d\n", n);
  memory_dispose();
  return 0;
}
