/*
 * $Id: zps.c,v 1.3 2002/10/14 19:11:51 jon Exp $
 *
 * Calculate permutation space representation
 *
 */

#include <stdio.h>
#include "endian.h"
#include "memory.h"
#include "parse.h"
#include "ps.h"

static const char *name = "zps";

static void ps_usage(void)
{
  fprintf(stderr, "%s: usage: %s [-v] [-m <memory>] <range> <image> <output>\n", name, name);
}

int main(int argc, const char * const argv[])
{
  argv = parse_line(argc, argv, &argc);
  if (4 != argc) {
    ps_usage();
    exit(1);
  }
  endian_init();
  memory_init(name, memory);
  permutation_space(argv[1], argv[2], argv[3], name);
  memory_dispose();
  return 0;
}
