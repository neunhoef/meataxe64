/*
 * $Id: ztspf.c,v 1.1 2002/06/25 10:30:12 jon Exp $
 *
 * Spin some vectors under two generators in tensor space
 * using intermediate files in a temporary directory
 *
 */

#include <stdio.h>
#include "endian.h"
#include "memory.h"
#include "tspf.h"

static const char *name = "ztspf";

static void tspf_usage(void)
{
  fprintf(stderr, "%s: usage: %s <in_file> <out_file> <gen_a1> <gen_a2> <gen_b1> <gen_b2> <tmp dir> [<memory>]\n", name, name);
}

int main(int argc, const char * const argv[])
{
  unsigned int memory = MEM_SIZE;
  unsigned int dim;

  if (8 != argc && 9 != argc) {
    tspf_usage();
    exit(1);
  }
  endian_init();
  if (9 == argc) {
    memory = strtoul(argv[8], NULL, 0);
  }
  memory_init(name, memory);
  dim = spin(argv[1], argv[2], argv[3], argv[4], argv[5], argv[6], argv[7], name);
  printf("%d\n", dim);
  memory_dispose();
  return 0;
}
