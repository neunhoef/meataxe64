/*
 * $Id: zspf.c,v 1.2 2002/05/26 00:47:20 jon Exp $
 *
 * Spin some vectors under two generators to obtain a standard base
 * using intermediate in a temporary directory
 *
 */

#include <stdio.h>
#include "endian.h"
#include "memory.h"
#include "spf.h"

static const char *name = "zspf";

static void spf_usage(void)
{
  fprintf(stderr, "%s: usage: %s <in_file> <out_file> <gen_a> <gen_b> <tmp dir> [<memory>]\n", name, name);
}

int main(int argc, const char * const argv[])
{
  unsigned int memory = MEM_SIZE;
  unsigned int dim;

  if (6 != argc && 7 != argc) {
    spf_usage();
    exit(1);
  }
  endian_init();
  if (7 == argc) {
    memory = strtoul(argv[6], NULL, 0);
  }
  memory_init(name, memory);
  dim = spin(argv[1], argv[2], argv[3], argv[4], argv[5], name);
  printf("%d\n", dim);
  memory_dispose();
  return 0;
}
