/*
 * $Id: ztsp.c,v 1.7 2005/06/22 21:52:55 jon Exp $
 *
 * Spin some vectors under two generators in tensor space
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include "endian.h"
#include "memory.h"
#include "parse.h"
#include "tsp.h"

static const char *name = "ztsp";

static void tsp_usage(void)
{
  fprintf(stderr, "%s: usage: %s %s <in_file> <out_file> <gen_a1> <gen_a2> <gen_b1> <gen_b2>\n", name, name, parse_usage());
}

int main(int argc, const char * const argv[])
{
  u32 dim;

  argv = parse_line(argc, argv, &argc);
  if (7 != argc) {
    tsp_usage();
    exit(1);
  }
  endian_init();
  memory_init(name, memory);
  dim = tensor_spin(argv[1], argv[2], argv[3], argv[4], argv[5], argv[6], name);
  printf("%d\n", dim);
  memory_dispose();
  return 0;
}
