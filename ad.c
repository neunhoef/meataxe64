/*
 * $Id: ad.c,v 1.4 2001/09/16 20:20:39 jon Exp $
 *
 * Add two matrices to give a third
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include "memory.h"
#include "add.h"

static const char *name = "ad";

static void ad_usage(void)
{
  fprintf(stderr, "%s: usage: %s <in_file> <in_file> <out_file>\n", name, name);
}

int main(int argc, const char * const argv[])
{
  const char *in1;
  const char *in2;
  const char *out;

  if (4 != argc) {
    ad_usage();
    exit(1);
  }
  in1 = argv[1];
  in2 = argv[2];
  out = argv[3];
  memory_init(name, 0);
  if (0 == add(in1, in2, out, name)) {
    exit(1);
  }
  memory_dispose();
  return 0;
}
