/*
 * $Id: zrnf.c,v 1.6 2002/10/14 09:05:57 jon Exp $
 *
 * Compute the rank of a matrix, using temporary files
 *
 */

#include <stdio.h>
#include "endian.h"
#include "memory.h"
#include "parse.h"
#include "rnf.h"

static const char *name = "zrnf";

static void rnf_usage(void)
{
  fprintf(stderr, "%s: usage: %s [-v] [-m <memory>] <in_file> <temp dir>\n", name, name);
}

int main(int argc, const char * const argv[])
{
  unsigned int n;

  argv = parse_line(argc, argv, &argc);
  if (3 != argc) {
    rnf_usage();
    exit(1);
  }
  endian_init();
  memory_init(name, memory);
  n = rank(argv[1], argv[2], name);
  printf("%d\n", n);
  memory_dispose();
  return 0;
}
