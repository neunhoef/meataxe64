/*
 * $Id: zspanmsp.c,v 1.2 2002/10/14 19:11:51 jon Exp $
 *
 * Compute subspaces from the span of a matrix until we find a proper subspace
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "endian.h"
#include "memory.h"
#include "parse.h"
#include "spanmsp.h"

static const char *name = "zspanmsp";

static void spanmsp_usage(void)
{
  fprintf(stderr, "%s: usage: %s [-v] [-m <memory>] <in_file> <out_file> <gen_1> [<gen>*]\n", name, name);
}

int main(int argc, const char * const argv[])
{
  const char *in;
  const char *out;
  unsigned int i;

  argv = parse_line(argc, argv, &argc);
  if (4 <= argc) {
    spanmsp_usage();
    exit(1);
  }
  endian_init();
  memory_init(name, memory);
  in = argv[1];
  out = argv[2];
  i = spanmspin(in, out, argc - 3, argv + 3, name);
  if (1 == i ) {
    return 0;
  } else if (255 == i) {
    return 255;
  } else {
    return 1;
  }
}
