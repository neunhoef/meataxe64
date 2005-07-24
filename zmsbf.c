/*
 * $Id: zmsbf.c,v 1.9 2005/07/24 09:32:46 jon Exp $
 *
 * Spin some vectors under multiple generators to obtain a standard base
 * using intermediate files in a temporary directory
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include "endian.h"
#include "memory.h"
#include "msbf.h"
#include "parse.h"

static const char *name = "zmsbf";

static void msbf_usage(void)
{
  fprintf(stderr, "%s: usage: %s %s <in_file> <out_file> <tmp dir> <gen_1> [<gen>*]\n", name, name, parse_usage());
}

int main(int argc, const char * const argv[])
{
  u32 dim;

  argv = parse_line(argc, argv, &argc);
  if (5 > argc) {
    msbf_usage();
    exit(1);
  }
  endian_init();
  memory_init(name, memory);
  dim = msb_spinf(argv[1], argv[2], argv[3], argc - 4, argv + 4, name);
  printf("%u\n", dim);
  memory_dispose();
  return 0;
}
