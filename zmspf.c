/*
 * $Id: zmspf.c,v 1.4 2002/10/14 09:20:33 jon Exp $
 *
 * Spin some vectors under multiple generators, using intermediate files
 *
 */

#include <stdio.h>
#include "endian.h"
#include "memory.h"
#include "mspf.h"
#include "parse.h"

static const char *name = "zmspf";

static void mspf_usage(void)
{
  fprintf(stderr, "%s: usage: %s [-v] [-m <memory>] <in_file> <out_file> <tmp dir> <gen_1> [<gen>*]\n", name, name);
}

int main(int argc, const char * const argv[])
{
  unsigned int dim;

  argv = parse_line(argc, argv, &argc);
  if (5 > argc) {
    mspf_usage();
    exit(1);
  }
  endian_init();
  memory_init(name, memory);
  dim = spin(argv[1], argv[2], argv[3], argc - 4, argv + 4, name);
  printf("%d\n", dim);
  memory_dispose();
  return 0;
}
