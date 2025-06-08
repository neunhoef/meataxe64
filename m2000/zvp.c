/*
 * $Id: zvp.c,v 1.9 2005/07/24 09:32:46 jon Exp $
 *
 * Permute some vectors under two generators
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include "endian.h"
#include "memory.h"
#include "parse.h"
#include "vp.h"

static const char *name = "zvp";

static void vp_usage(void)
{
  fprintf(stderr, "%s: usage: %s %s <in_file> <out_file> <gen_a> <gen_b> <out a> <out b>\n", name, name, parse_usage());
}

int main(int argc, const char * const argv[])
{
  u32 degree;

  argv = parse_line(argc, argv, &argc);
  if (7 != argc) {
    vp_usage();
    exit(1);
  }
  endian_init();
  memory_init(name, memory);
  degree = permute(argv[1], argv[2], argv[3], argv[4], argv[5], argv[6], 0, name);
  printf("%u\n", degree);
  memory_dispose();
  return 0;
}
