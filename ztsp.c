/*
 * $Id: ztsp.c,v 1.2 2002/07/09 09:08:12 jon Exp $
 *
 * Spin some vectors under two generators in tensor space
 *
 */

#include <stdio.h>
#include "endian.h"
#include "memory.h"
#include "parse.h"
#include "tsp.h"

static const char *name = "ztsp";

static void tsp_usage(void)
{
  fprintf(stderr, "%s: usage: %s <in_file> <out_file> <gen_a1> <gen_a2> <gen_b1> <gen_b2> [<memory>]\n", name, name);
}

int main(int argc, const char * const argv[])
{
  unsigned int memory = MEM_SIZE;
  unsigned int dim;

  argv = parse_line(argc, argv, &argc);
  if (7 != argc && 8 != argc) {
    tsp_usage();
    exit(1);
  }
  endian_init();
  if (8 == argc) {
    memory = strtoul(argv[7], NULL, 0);
  }
  memory_init(name, memory);
  dim = spin(argv[1], argv[2], argv[3], argv[4], argv[5], argv[6], name);
  printf("%d\n", dim);
  memory_dispose();
  return 0;
}
