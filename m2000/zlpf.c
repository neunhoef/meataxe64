/*
 * $Id: zlpf.c,v 1.2 2005/07/24 09:32:46 jon Exp $
 *
 * Projective (line) permute some vectors under two generators,
 * using intermediate file
 */

#include <stdio.h>
#include <stdlib.h>
#include "endian.h"
#include "memory.h"
#include "parse.h"
#include "vpf.h"

static const char *name = "zlpf";

static void lpf_usage(void)
{
  fprintf(stderr, "%s: usage: %s %s <tmp_dir> <in_file> <out_file> <gen_a> <gen_b> <out a> <out b>\n", name, name, parse_usage());
}

int main(int argc, const char * const argv[])
{
  unsigned int degree;

  argv = parse_line(argc, argv, &argc);
  if (8 != argc) {
    lpf_usage();
    exit(1);
  }
  endian_init();
  memory_init(name, memory);
  degree = permute_file(argv[1], argv[2], argv[3], argv[4], argv[5], argv[6], argv[7], 1, name);
  printf("%u\n", degree);
  memory_dispose();
  return 0;
}
