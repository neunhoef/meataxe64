/*
 * $Id: zsymb.c,v 1.3 2004/08/21 13:22:32 jon Exp $
 *
 * Create a symmetry basis
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include "endian.h"
#include "memory.h"
#include "symb.h"
#include "parse.h"

static const char *name = "zsymb";

static void symb_usage(void)
{
  fprintf(stderr, "%s: usage: %s %s <number of spaces> <expected space size> <in_file> <out_file> <tmp dir> <gen_1> [<gen>*]\n", name, name, parse_usage());
}

int main(int argc, const char * const argv[])
{
  unsigned int dim;
  unsigned int spaces, size;

  argv = parse_line(argc, argv, &argc);
  if (6 > argc) {
    symb_usage();
    exit(1);
  }
  endian_init();
  memory_init(name, memory);
  spaces = strtoul(argv[1], NULL, 0);
  size = strtoul(argv[2], NULL, 0);
  dim = symb(spaces, size, argv[3], argv[4], argv[5], argc - 6, argv + 6, name);
  printf("%d\n", dim);
  memory_dispose();
  return 0;
}
