/*
 * $Id: tr.c,v 1.7 2004/01/04 21:22:50 jon Exp $
 *
 * Transpose a matrix
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include "endian.h"
#include "memory.h"
#include "parse.h"
#include "tra.h"

static const char *name = "ztr";

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
  if (0 == tra(in, out, name)) {
    exit(1);
  }
  memory_dispose();
  return 0;
}
