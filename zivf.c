/*
 * $Id: zivf.c,v 1.3 2002/10/14 09:10:31 jon Exp $
 *
 * Invert a matrix
 *
 */

#include <stdio.h>
#include "endian.h"
#include "ivf.h"
#include "memory.h"
#include "parse.h"

static const char *name = "zivf";

static void ivf_usage(void)
{
  fprintf(stderr, "%s: usage: %s [-v] [-m<memory>] <in_file> <out_file> <temp dir>\n", name, name);
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
