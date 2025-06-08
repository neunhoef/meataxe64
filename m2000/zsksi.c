/*
 * $Id: zsksi.c,v 1.4 2004/01/04 21:22:50 jon Exp $
 *
 * Skew sixth a matrix
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "endian.h"
#include "memory.h"
#include "parse.h"
#include "powers.h"
#include "utils.h"

static const char *name = "zsksi";

static void sksi_usage(void)
{
  fprintf(stderr, "%s: usage: %s %s <in_file> <out_file>\n", name, name, parse_usage());
}

int main(int argc, const char * const argv[])
{
  const char *in;
  const char *out;

  argv = parse_line(argc, argv, &argc);
  if (3 != argc) {
    sksi_usage();
    exit(1);
  }
  in = argv[1];
  out = argv[2];
  endian_init();
  memory_init(name, memory);
  if (0 == skew_sixth(in, out, name)) {
    exit(1);
  }
  memory_dispose();
  return 0;
}
