/*
 * $Id: zmsp.c,v 1.3 2002/10/14 09:20:33 jon Exp $
 *
 * Spin some vectors under multiple generators
 *
 */

#include <stdio.h>
#include "endian.h"
#include "memory.h"
#include "msp.h"
#include "parse.h"

static const char *name = "zmsp";

static void msp_usage(void)
{
  fprintf(stderr, "%s: usage: %s [-v] [-m <memory>] <in_file> <out_file> <gen_1> [<gen>*]\n", name, name);
}

int main(int argc, const char * const argv[])
{
  unsigned int dim;

  argv = parse_line(argc, argv, &argc);
  if (4 > argc) {
    msp_usage();
    exit(1);
  }
  endian_init();
  memory_init(name, memory);
  dim = spin(argv[1], argv[2], argc - 3, argv + 3, name);
  printf("%d\n", dim);
  memory_dispose();
  return 0;
}
