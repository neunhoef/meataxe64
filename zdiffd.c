/*
 * $Id: zdiffd.c,v 1.3 2004/01/04 21:22:50 jon Exp $
 *
 * Find the differences between two matrices
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include "diffd.h"
#include "endian.h"
#include "memory.h"
#include "parse.h"

static const char *name = "zdiffd";

static void diff_usage(void)
{
  fprintf(stderr, "%s: usage: %s %s <in_file> <elt>\n", name, name, parse_usage());
}

int main(int argc, const char * const argv[])
{
  const char *in1;
  unsigned int elt;

  argv = parse_line(argc, argv, &argc);
  if (3 != argc) {
    diff_usage();
    exit(1);
  }
  in1 = argv[1];
  elt = strtoul(argv[2], NULL, 0);
  memory_init(name, 0);
  endian_init();
  if (0 == diffd(in1, elt, name)) {
    exit(255);
  }
  memory_dispose();
  return 0;
}
