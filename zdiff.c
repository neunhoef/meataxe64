/*
 * $Id: zdiff.c,v 1.1 2001/12/15 20:47:27 jon Exp $
 *
 * Find the differences between two matrices
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include "memory.h"
#include "diff.h"

static const char *name = "zdiff";

static void diff_usage(void)
{
  fprintf(stderr, "%s: usage: %s <in_file> <in_file> <out_file>\n", name, name);
}

int main(int argc, const char * const argv[])
{
  const char *in1;
  const char *in2;

  if (3 != argc) {
    diff_usage();
    exit(1);
  }
  in1 = argv[1];
  in2 = argv[2];
  memory_init(name, 0);
  if (0 == diff(in1, in2, name)) {
    exit(1);
  }
  memory_dispose();
  return 0;
}
