/*
 * $Id: zsymsq.c,v 1.3 2002/10/14 19:11:51 jon Exp $
 *
 * Symmetric square a matrix
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "endian.h"
#include "memory.h"
#include "parse.h"
#include "powers.h"
#include "utils.h"

static const char *name = "zsymsq";

static void symsq_usage(void)
{
  fprintf(stderr, "%s: usage: %s [-v] [-m <memory>] <in_file> <out_file>\n", name, name);
}

int main(int argc, const char * const argv[])
{
  const char *in;
  const char *out;

  argv = parse_line(argc, argv, &argc);
  if (3 != argc) {
    symsq_usage();
    exit(1);
  }
  in = argv[1];
  out = argv[2];
  endian_init();
  memory_init(name, memory);
  if (0 == sym_square(in, out, name)) {
    exit(1);
  }
  memory_dispose();
  return 0;
}
