/*
 * $Id: zsb.c,v 1.1 2001/12/23 23:31:42 jon Exp $
 *
 * Spin some vectors under two generators to obtain a standard base
 *
 */

#include <stdio.h>
#include "endian.h"
#include "memory.h"
#include "sb.h"

static const char *name = "zsb";

static void sb_usage(void)
{
  fprintf(stderr, "%s: usage: %s <in_file> <out_file> <gen_a> <gen_b> [<memory>]\n", name, name);
}

int main(int argc, const char * const argv[])
{
  unsigned int memory = MEM_SIZE;
  unsigned int dim;

  if (5 != argc && 6 != argc) {
    sb_usage();
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
