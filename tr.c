/*
 * $Id: tr.c,v 1.6 2002/10/13 19:09:42 jon Exp $
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
  fprintf(stderr, "%s: usage: %s [-v] [-m <memory>] <in_file> <out_file>\n", name, name);
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
