/*
 * $Id: zbase.c,v 1.6 2002/10/13 20:07:20 jon Exp $
 *
 * Compute a basis for a space from a set of vectors
 *
 */

#include <stdio.h>
#include "base.h"
#include "endian.h"
#include "memory.h"
#include "parse.h"

static const char *name = "zbase";

static void base_usage(void)
{
  fprintf(stderr, "%s: usage: %s [-v] [-m <memory>] <in_file> <temp dir> <output>\n", name, name);
}

int main(int argc, const char * const argv[])
{
  unsigned int n;

  argv = parse_line(argc, argv, &argc);
  if (4 != argc) {
    base_usage();
    exit(1);
  }
  endian_init();
  memory_init(name, memory);
  n = base(argv[1], argv[2], argv[3], name);
  printf("%d\n", n);
  memory_dispose();
  return 0;
}
