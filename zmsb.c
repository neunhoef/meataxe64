/*
 * $Id: zmsb.c,v 1.4 2004/01/04 21:22:50 jon Exp $
 *
 * Spin some vectors under two generators to obtain a standard base
 *
 */

#include <stdio.h>
#include "endian.h"
#include "memory.h"
#include "msb.h"
#include "parse.h"

static const char *name = "zmsb";

static void msb_usage(void)
{
  fprintf(stderr, "%s: usage: %s %s <in_file> <out_file> <gen_1> [<gen>*]\n", name, name, parse_usage());
}

int main(int argc, const char * const argv[])
{
  unsigned int dim;

  argv = parse_line(argc, argv, &argc);
  if (4 > argc) {
    msb_usage();
    exit(1);
  }
  endian_init();
  memory_init(name, memory);
  dim = spin(argv[1], argv[2], argc - 3, argv + 3, name);
  printf("%d\n", dim);
  memory_dispose();
  return 0;
}
