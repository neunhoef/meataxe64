/*
 * $Id: zvp.c,v 1.1 2002/01/26 00:36:06 jon Exp $
 *
 * Permute some vectors under two generators
 *
 */

#include <stdio.h>
#include "endian.h"
#include "memory.h"
#include "vp.h"

static const char *name = "zvp";

static void vp_usage(void)
{
  fprintf(stderr, "%s: usage: %s <in_file> <out_file> <gen_a> <gen_b> [<memory>]\n", name, name);
}

int main(int argc, const char * const argv[])
{
  unsigned int memory = MEM_SIZE;
  unsigned int degree;

  if (5 != argc && 6 != argc) {
    vp_usage();
    exit(1);
  }
  endian_init();
  if (6 == argc) {
    memory = strtoul(argv[5], NULL, 0);
  }
  memory_init(name, memory);
  degree = permute(argv[1], argv[2], argv[3], argv[4], name);
  printf("%d\n", degree);
  memory_dispose();
  return 0;
}
