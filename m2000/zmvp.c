/*
 * $Id: zmvp.c,v 1.1 2006/07/19 21:26:00 jon Exp $
 *
 * Permute some vectors under multiple generators
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include "endian.h"
#include "memory.h"
#include "parse.h"
#include "mvp.h"

static const char *name = "zmvp";

static void mvp_usage(void)
{
  fprintf(stderr, "%s: usage: %s %s <in_file> <out_file> <gen_1> [<gen>*]\n", name, name, parse_usage());
}

int main(int argc, const char * const argv[])
{
  u32 degree;

  argv = parse_line(argc, argv, &argc);
  if (4 > argc) {
    mvp_usage();
    exit(1);
  }
  endian_init();
  memory_init(name, memory);
  degree = multi_permute(argv[1], argv[2], argc - 3, argv + 3, 0, name);
  printf("%u\n", degree);
  memory_dispose();
  return 0;
}
