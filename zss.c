/*
 * $Id: zss.c,v 1.3 2002/10/13 22:41:39 jon Exp $
 *
 * Calculate subspace representation
 *
 */

#include <stdio.h>
#include "endian.h"
#include "memory.h"
#include "parse.h"
#include "ss.h"

static const char *name = "zss";

static void ss_usage(void)
{
  fprintf(stderr, "%s: usage: %s [-v] [-m <memory>] <range> <image> <output>\n", name, name);
}

int main(int argc, const char * const argv[])
{
  argv = parse_line(argc, argv, &argc);
  if (4 != argc) {
    ss_usage();
    exit(1);
  }
  endian_init();
  memory_init(name, memory);
  subspace(argv[1], argv[2], argv[3], name);
  memory_dispose();
  return 0;
}
