/*
 * $Id: ad.c,v 1.6 2002/01/06 16:35:48 jon Exp $
 *
 * Add two matrices to give a third
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include "endian.h"
#include "memory.h"
#include "add.h"

static const char *name = "zad";

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
  endian_init();
  if (0 == add(in1, in2, out, name)) {
    exit(1);
  }
  memory_dispose();
  return 0;
}
