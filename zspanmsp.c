/*
 * $Id: zspanmsp.c,v 1.1 2002/09/05 18:24:26 jon Exp $
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
  fprintf(stderr, "%s: usage: %s <in_file> <out_file> <memory> <gen_1> [<gen>*]\n", name, name);
}

int main(int argc, const char * const argv[])
{
  const char *in;
  const char *out;
  unsigned int memory = MEM_SIZE;
  unsigned int i;

  argv = parse_line(argc, argv, &argc);
  if (5 <= argc) {
    spanmsp_usage();
    exit(1);
  }
  memory = strtoul(argv[3], NULL, 0);
  endian_init();
  memory_init(name, memory);
  in = argv[1];
  out = argv[2];
  i = spanmspin(in, out, argc - 4, argv + 4, name);
  if (1 == i ) {
    return 0;
  } else if (255 == i) {
    return 255;
  } else {
    return 1;
  }
}
