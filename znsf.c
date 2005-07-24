/*
 * $Id: znsf.c,v 1.8 2005/07/24 09:32:46 jon Exp $
 *
 * Compute the nullspace of a matrix, using temporary files
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include "endian.h"
#include "memory.h"
#include "nsf.h"
#include "parse.h"

static const char *name = "znsf";

static void nsf_usage(void)
{
  fprintf(stderr, "%s: usage: %s %s <in_file> <out_file> <temp dir>\n", name, name, parse_usage());
}

int main(int argc, const char * const argv[])
{
  u32 n;

  argv = parse_line(argc, argv, &argc);
  if (4 != argc) {
    nsf_usage();
    exit(1);
  }
  endian_init();
  memory_init(name, memory);
  n = nullspacef(argv[1], argv[2], argv[3], name);
  printf("%u\n", n);
  memory_dispose();
  return 0;
}
