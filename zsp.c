/*
 * $Id: zsp.c,v 1.2 2001/11/22 20:04:03 jon Exp $
 *
 * Spin some vectors under two generators
 *
 */

#include <stdio.h>
#include "endian.h"
#include "memory.h"
#include "sp.h"

static const char *name = "zsp";

static void sp_usage(void)
{
  fprintf(stderr, "%s: usage: %s <in_file> <out_file> <gen_a> <gen_b_file> [<memory>]\n", name, name);
}

int main(int argc, const char * const argv[])
{
  unsigned int memory = MEM_SIZE;
  unsigned int dim;

  if (5 != argc && 6 != argc) {
    sp_usage();
    exit(1);
  }
  endian_init();
  if (6 == argc) {
    memory = strtoul(argv[5], NULL, 0);
  }
  memory_init(name, memory);
  dim = spin(argv[1], argv[2], argv[3], argv[4], name);
  printf("%d\n", dim);
  memory_dispose();
  return 0;
}
