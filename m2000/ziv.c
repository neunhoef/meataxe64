/*
 * $Id: ziv.c,v 1.5 2004/08/21 13:22:31 jon Exp $
 *
 * Invert a matrix
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include "endian.h"
#include "iv.h"
#include "memory.h"
#include "parse.h"

static const char *name = "ziv";

static void iv_usage(void)
{
  fprintf(stderr, "%s: usage: %s %s <in_file> <out_file>\n", name, name, parse_usage());
}

int main(int argc, const char * const argv[])
{
  argv = parse_line(argc, argv, &argc);
  if (3 != argc) {
    iv_usage();
    exit(1);
  }
  endian_init();
  memory_init(name, memory);
  invert(argv[1], argv[2], name);
  memory_dispose();
  return 0;
}
