/*
 * $Id: zmsbf.c,v 1.3 2002/10/14 09:05:57 jon Exp $
 *
 * Spin some vectors under multiple generators to obtain a standard base
 * using intermediate files in a temporary directory
 *
 */

#include <stdio.h>
#include "endian.h"
#include "memory.h"
#include "msbf.h"
#include "parse.h"

static const char *name = "zmsbf";

static void msbf_usage(void)
{
  fprintf(stderr, "%s: usage: %s [-v] [-m <memory>] <in_file> <out_file> <tmp dir> <gen_1> [<gen>*]\n", name, name);
}

int main(int argc, const char * const argv[])
{
  unsigned int dim;

  argv = parse_line(argc, argv, &argc);
  if (5 > argc) {
    msbf_usage();
    exit(1);
  }
  endian_init();
  memory = strtoul(argv[4], NULL, 0);
  memory_init(name, memory);
  dim = spin(argv[1], argv[2], argv[3], argc - 4, argv + 4, name);
  printf("%d\n", dim);
  memory_dispose();
  return 0;
}
