/*
 * $Id: zjoin.c,v 1.3 2004/01/04 21:22:50 jon Exp $
 *
 * Append two matrices to give a third
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include "endian.h"
#include "join.h"
#include "memory.h"
#include "parse.h"

static const char *name = "zjoin";

static void join_usage(void)
{
  fprintf(stderr, "%s: usage: %s %s <in_file> <in_file> <out_file>\n", name, name, parse_usage());
}

int main(int argc, const char * const argv[])
{
  const char *in1;
  const char *in2;
  const char *out;

  argv = parse_line(argc, argv, &argc);
  if (4 != argc) {
    join_usage();
    exit(1);
  }
  in1 = argv[1];
  in2 = argv[2];
  out = argv[3];
  memory_init(name, 0);
  endian_init();
  if (0 == join(in1, in2, out, name)) {
    exit(1);
  }
  memory_dispose();
  return 0;
}
