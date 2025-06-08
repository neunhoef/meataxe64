/*
 * $Id: zpofp.c,v 1.1 2006/03/12 10:23:19 jon Exp $
 *
 * Find fixed points of permutation in orbit
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include "endian.h"
#include "memory.h"
#include "parse.h"
#include "pofp.h"

static const char *name = "zpofp";

static void pofp_usage(void)
{
  fprintf(stderr, "%s: usage: %s %s <orbit set> <orbit number> <output> <permutation> [<other permutations.]\n", name, name, parse_usage());
}

int main(int argc, const char * const argv[])
{
  u32 res;
  unsigned int orbit_num;
  endian_init();
  argv = parse_line(argc, argv, &argc);
  if (5 > argc) {
    pofp_usage();
    exit(1);
  }
  orbit_num = strtoul(argv[2], NULL, 0);
  endian_init();
  memory_init(name, memory);
  res = fixed_points_orbit(argv[1], orbit_num, argv[3], argc - 4, argv + 4, name);
  memory_dispose();
  printf("%u\n", res);
  return 0;
}
