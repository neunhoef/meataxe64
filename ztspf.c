/*
 * $Id: ztspf.c,v 1.6 2004/08/28 19:58:01 jon Exp $
 *
 * Spin some vectors under two generators in tensor space
 * using intermediate files in a temporary directory
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include "endian.h"
#include "memory.h"
#include "parse.h"
#include "tspf.h"

static const char *name = "ztspf";

static void tspf_usage(void)
{
  fprintf(stderr, "%s: usage: %s %s <in_file> <out_file> <gen_a1> <gen_a2> <gen_b1> <gen_b2> <tmp dir>\n", name, name, parse_usage());
}

int main(int argc, const char * const argv[])
{
  unsigned int dim;

  argv = parse_line(argc, argv, &argc);
  if (8 != argc) {
    tspf_usage();
    exit(1);
  }
  endian_init();
  memory_init(name, memory);
  dim = tensor_spinf(argv[1], argv[2], argv[3], argv[4], argv[5], argv[6], argv[7], name);
  printf("%d\n", dim);
  memory_dispose();
  return 0;
}
