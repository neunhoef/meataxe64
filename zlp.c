/*
 * $Id: zlp.c,v 1.2 2002/07/09 09:08:12 jon Exp $
 *
 * Projective (line) permute some vectors under two generators
 *
 */

#include <stdio.h>
#include "endian.h"
#include "memory.h"
#include "parse.h"
#include "vp.h"

static const char *name = "zlp";

static void lp_usage(void)
{
  fprintf(stderr, "%s: usage: %s <in_file> <out_file> <gen_a> <gen_b> <out a> <out b> [<memory>]\n", name, name);
}

int main(int argc, const char * const argv[])
{
  unsigned int memory = MEM_SIZE;
  unsigned int degree;

  argv = parse_line(argc, argv, &argc);
  if (7 != argc && 8 != argc) {
    lp_usage();
    exit(1);
  }
  endian_init();
  if (8 == argc) {
    memory = strtoul(argv[7], NULL, 0);
  }
  memory_init(name, memory);
  degree = permute(argv[1], argv[2], argv[3], argv[4], argv[5], argv[6], 1, name);
  printf("%d\n", degree);
  memory_dispose();
  return 0;
}
