/*
 * $Id: znsf.c,v 1.3 2002/10/14 09:05:57 jon Exp $
 *
 * Compute the nullspace of a matrix, using temporary files
 *
 */

#include <stdio.h>
#include "endian.h"
#include "memory.h"
#include "nsf.h"
#include "parse.h"

static const char *name = "znsf";

static void nsf_usage(void)
{
  fprintf(stderr, "%s: usage: %s [-v] [-m <memory>] <in_file> <out_file> <temp dir>\n", name, name);
}

int main(int argc, const char * const argv[])
{
  unsigned int n;

  argv = parse_line(argc, argv, &argc);
  if (4 != argc) {
    nsf_usage();
    exit(1);
  }
  endian_init();
  memory_init(name, memory);
  n = nullspace(argv[1], argv[2], argv[3], name);
  printf("%d\n", n);
  memory_dispose();
  return 0;
}
