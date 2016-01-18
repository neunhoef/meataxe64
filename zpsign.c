/*
 * Compute the sign of a permutation
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include "endian.h"
#include "memory.h"
#include "psign.h"
#include "parse.h"

static const char *name = "zpsign";

static void psign_usage(void)
{
  fprintf(stderr, "%s: usage: %s %s <permutation>\n", name, name, parse_usage());
}

int main(int argc, const char * const argv[])
{
  int n;

  argv = parse_line(argc, argv, &argc);
  if (3 != argc) {
    psign_usage();
    exit(1);
  }
  endian_init();
  memory_init(name, memory);
  n = psign(argv[1], name);
  memory_dispose();
  return n;
}
