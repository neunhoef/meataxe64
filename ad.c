/*
 * $Id: ad.c,v 1.3 2001/09/04 23:00:12 jon Exp $
 *
 * Add two matrices to give a third
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include "add.h"

static void ad_usage(void)
{
  fprintf(stderr, "ad: usage: ad <in_file> <in_file> <out_file>\n");
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
  if (0 == add(in1, in2, out, "ad")) {
    exit(1);
  }
  return 0;
}
