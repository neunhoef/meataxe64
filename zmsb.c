/*
 * $Id: zmsb.c,v 1.1 2002/07/05 12:43:41 jon Exp $
 *
 * Spin some vectors under two generators to obtain a standard base
 *
 */

#include <stdio.h>
#include "endian.h"
#include "memory.h"
#include "msb.h"

static const char *name = "zmsb";

static void msb_usage(void)
{
  fprintf(stderr, "%s: usage: %s <in_file> <out_file> <memory> <gen_1> [<gen>*]\n", name, name);
}

int main(int argc, const char * const argv[])
{
  unsigned int memory = MEM_SIZE;
  unsigned int dim;

  if (5 > argc) {
    msb_usage();
    exit(1);
  }
  endian_init();
  memory = strtoul(argv[3], NULL, 0);
  memory_init(name, memory);
  dim = spin(argv[1], argv[2], argc - 4, argv + 4, name);
  printf("%d\n", dim);
  memory_dispose();
  return 0;
}
