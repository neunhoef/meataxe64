/*
 * $Id: zmspf.c,v 1.8 2005/06/22 21:52:54 jon Exp $
 *
 * Spin some vectors under multiple generators, using intermediate files
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include "endian.h"
#include "memory.h"
#include "mspf.h"
#include "parse.h"

static const char *name = "zmspf";

static void mspf_usage(void)
{
  fprintf(stderr, "%s: usage: %s %s <in_file> <out_file> <tmp dir> <gen_1> [<gen>*]\n", name, name, parse_usage());
}

int main(int argc, const char * const argv[])
{
  u32 dim;

  argv = parse_line(argc, argv, &argc);
  if (5 > argc) {
    mspf_usage();
    exit(1);
  }
  endian_init();
  memory_init(name, memory);
  dim = spinf(argv[1], argv[2], argv[3], argc - 4, argv + 4, name);
  printf("%d\n", dim);
  memory_dispose();
  return 0;
}
