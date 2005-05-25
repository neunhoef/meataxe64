/*
 * $Id: zvpf.c,v 1.1 2005/05/25 18:35:56 jon Exp $
 *
 * Permute some vectors under two generators,
 * using intermediate file
 */

#include <stdio.h>
#include <stdlib.h>
#include "endian.h"
#include "memory.h"
#include "parse.h"
#include "vpf.h"

static const char *name = "zvpf";

static void vpf_usage(void)
{
  fprintf(stderr, "%s: usage: %s %s <tmp_dir> <in_file> <out_file> <gen_a> <gen_b> <out a> <out b>\n", name, name, parse_usage());
}

int main(int argc, const char * const argv[])
{
  unsigned int degree;

  argv = parse_line(argc, argv, &argc);
  if (8 != argc) {
    vpf_usage();
    exit(1);
  }
  endian_init();
  memory_init(name, memory);
  degree = permute_file(argv[1], argv[2], argv[3], argv[4], argv[5], argv[6], argv[7], 0, name);
  printf("%d\n", degree);
  memory_dispose();
  return 0;
}
