/*
 * $Id: zdiff.c,v 1.5 2004/01/04 21:22:50 jon Exp $
 *
 * Find the differences between two matrices
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include "diff.h"
#include "endian.h"
#include "memory.h"
#include "parse.h"

static const char *name = "zdiff";

static void diff_usage(void)
{
  fprintf(stderr, "%s: usage: %s %s <in_file> <in_file>\n", name, name, parse_usage());
}

int main(int argc, const char * const argv[])
{
  const char *in1;
  const char *in2;

  argv = parse_line(argc, argv, &argc);
  if (3 != argc) {
    diff_usage();
    exit(1);
  }
  in1 = argv[1];
  in2 = argv[2];
  memory_init(name, 0);
  endian_init();
  if (0 == diff(in1, in2, name)) {
    exit(1);
  }
  memory_dispose();
  return 0;
}
