/*
 * $Id: zrestrict.c,v 1.1 2002/03/20 18:42:30 jon Exp $
 *
 * Restrict a matrix to a subfield
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include "endian.h"
#include "memory.h"
#include "restrict.h"

static const char *name = "zrestrict";

static void restrict_usage(void)
{
  fprintf(stderr, "%s: usage: %s <in_file> <out_file> <field order>\n", name, name);
}

int main(int argc, const char * const argv[])
{
  const char *in;
  const char *out;
  unsigned int q, r;

  if (4 != argc) {
    restrict_usage();
    exit(1);
  }
  in = argv[1];
  out = argv[2];
  q = strtoul(argv[3], NULL, 0);
  memory_init(name, 0);
  endian_init();
  r = restrict(in, out, q, name);
  if (1 != r) {
    exit((0 == r) ? 1 : 255);
  }
  memory_dispose();
  return 0;
}
