/*
 * $Id: mu.c,v 1.11 2002/10/13 14:16:07 jon Exp $
 *
 * Multiply two matrices to give a third
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "endian.h"
#include "memory.h"
#include "mul.h"
#include "parse.h"
#include "utils.h"

static const char *name = "zmu";

static void mu_usage(void)
{
  fprintf(stderr, "%s: usage: %s [-v] [-m <memory>] <in_file> <in_file> <out_file>\n", name, name);
}

int main(int argc, const char * const argv[])
{
  const char *in1;
  const char *in2;
  const char *out;

  argv = parse_line(argc, argv, &argc);
  if (4 != argc) {
    mu_usage();
    exit(1);
  }
  in1 = argv[1];
  in2 = argv[2];
  out = argv[3];
  memory_init(name, memory);
  endian_init();
  if (0 == mul(in1, in2, out, name)) {
    exit(1);
  }
  memory_dispose();
  return 0;
}
