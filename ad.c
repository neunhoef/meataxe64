/*
 * $Id: ad.c,v 1.9 2004/01/31 13:24:51 jon Exp $
 *
 * Add two matrices to give a third
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include "add.h"
#include "endian.h"
#include "memory.h"
#include "parse.h"

static const char *name = "zad";

static void ad_usage(void)
{
  fprintf(stderr, "%s: usage: %s %s <in_file> <in_file> <out_file>\n", name, name, parse_usage());
}

int main(int argc, const char * const argv[])
{
  const char *in1;
  const char *in2;
  const char *out;

  argv = parse_line(argc, argv, &argc);
  if (4 != argc) {
    ad_usage();
    exit(1);
  }
  in1 = argv[1];
  in2 = argv[2];
  out = argv[3];
  memory_init(name, memory);
  endian_init();
  if (0 == add(in1, in2, out, name)) {
    exit(1);
  }
  memory_dispose();
  return 0;
}
