/*
 * $Id: zte.c,v 1.4 2002/10/14 19:11:51 jon Exp $
 *
 * Tensor two matrices to give a third
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "endian.h"
#include "memory.h"
#include "parse.h"
#include "te.h"
#include "utils.h"

static const char *name = "zte";

static void te_usage(void)
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
    te_usage();
    exit(1);
  }
  in1 = argv[1];
  in2 = argv[2];
  out = argv[3];
  endian_init();
  memory_init(name, memory);
  if (0 == tensor(in1, in2, out, name)) {
    exit(1);
  }
  memory_dispose();
  return 0;
}
