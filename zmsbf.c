/*
 * $Id: zmsbf.c,v 1.1 2002/07/07 12:10:42 jon Exp $
 *
 * Spin some vectors under multiple generators to obtain a standard base
 * using intermediate files in a temporary directory
 *
 */

#include <stdio.h>
#include "endian.h"
#include "memory.h"
#include "msbf.h"

static const char *name = "zmsbf";

static void msbf_usage(void)
{
  fprintf(stderr, "%s: usage: %s <in_file> <out_file> <tmp dir> <memory> <gen_1> [<gen>*]\n", name, name);
}

int main(int argc, const char * const argv[])
{
  unsigned int memory = MEM_SIZE;
  unsigned int dim;

  if (6 > argc) {
    msbf_usage();
    exit(1);
  }
  endian_init();
  if (7 == argc) {
    memory = strtoul(argv[6], NULL, 0);
  }
  memory = strtoul(argv[4], NULL, 0);
  memory_init(name, memory);
  dim = spin(argv[1], argv[2], argv[3], argc - 5, argv + 5, name);
  printf("%d\n", dim);
  memory_dispose();
  return 0;
}
