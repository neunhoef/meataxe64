/*
 * $Id: znsf.c,v 1.1 2002/01/18 21:52:23 jon Exp $
 *
 * Compute the nullspace of a matrix, using temporary files
 *
 */

#include <stdio.h>
#include "endian.h"
#include "memory.h"
#include "nsf.h"

static const char *name = "znsf";

static void nsf_usage(void)
{
  fprintf(stderr, "%s: usage: %s <in_file> <out_file> <temp dir> [<memory>]\n", name, name);
}

int main(int argc, const char * const argv[])
{
  unsigned int n;
  unsigned int memory = MEM_SIZE;

  if (4 != argc && 5 != argc) {
    nsf_usage();
    exit(1);
  }
  endian_init();
  if (5 == argc) {
    memory = strtoul(argv[4], NULL, 0);
  }
  memory_init(name, memory);
  n = nullspace(argv[1], argv[2], argv[3], name);
  printf("%d\n", n);
  memory_dispose();
  return 0;
}
