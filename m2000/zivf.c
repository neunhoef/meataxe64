/*
 * $Id: zivf.c,v 1.5 2004/08/21 13:22:31 jon Exp $
 *
 * Invert a matrix
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include "endian.h"
#include "ivf.h"
#include "memory.h"
#include "parse.h"

static const char *name = "zivf";

static void ivf_usage(void)
{
  fprintf(stderr, "%s: usage: %s %s <in_file> <out_file> <temp dir>\n", name, name, parse_usage());
}

int main(int argc, const char * const argv[])
{
  argv = parse_line(argc, argv, &argc);
  if (4 != argc) {
    ivf_usage();
    exit(1);
  }
  endian_init();
  memory_init(name, memory);
  invert(argv[1], argv[2], argv[3], name);
  memory_dispose();
  return 0;
}
