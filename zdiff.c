/*
 * $Id: zdiff.c,v 1.2 2002/01/06 16:35:48 jon Exp $
 *
 * Find the differences between two matrices
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include "diff.h"
#include "endian.h"
#include "memory.h"

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
  endian_init();
  if (0 == diff(in1, in2, name)) {
    exit(1);
  }
  memory_dispose();
  return 0;
}
