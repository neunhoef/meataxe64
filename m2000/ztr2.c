/*
 * $Id: ztr2.c,v 1.1 2012/02/07 22:00:45 jon Exp $
 *
 * Transpose a matrix - new implementation
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include "endian.h"
#include "memory.h"
#include "parse.h"
#include "tra2.h"

static const char *name = "ztr2";

static void tr_usage(void)
{
  fprintf(stderr, "%s: usage: %s %s <in_file> <out_file>\n", name, name, parse_usage());
}

int main(int argc, const char * const argv[])
{
  const char *in;
  const char *out;

  argv = parse_line(argc, argv, &argc);
  if (3 != argc) {
    tr_usage();
    exit(1);
  }
  in = argv[1];
  out = argv[2];
  memory_init(name, memory);
  endian_init();
  if (0 == tra2(in, out, name)) {
    exit(1);
  }
  memory_dispose();
  return 0;
}
