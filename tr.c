/*
 * $Id: tr.c,v 1.2 2001/10/16 22:55:53 jon Exp $
 *
 * Transpose a matrix
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include "memory.h"
#include "tra.h"

static const char *name = "ztr";

static void tr_usage(void)
{
  fprintf(stderr, "%s: usage: %s <in_file> <out_file>\n", name, name);
}

int main(int argc, const char * const argv[])
{
  const char *in;
  const char *out;

  if (3 != argc) {
    tr_usage();
    exit(1);
  }
  in = argv[1];
  out = argv[2];
  memory_init(name, 0);
  if (0 == tra(in, out, name)) {
    exit(1);
  }
  memory_dispose();
  return 0;
}
