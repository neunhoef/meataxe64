/*
 * Compute the determinant of a matrix mod field squares, using intermediate files
 * Returns 0 for is a square, 255 for is not a square
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include "endian.h"
#include "memory.h"
#include "dmsf.h"
#include "parse.h"

static const char *name = "zdmsf";

static void dmsf_usage(void)
{
  fprintf(stderr, "%s: usage: %s %s <matrix> <temporary dir> \n", name, name, parse_usage());
}

int main(int argc, const char * const argv[])
{
  int n;

  argv = parse_line(argc, argv, &argc);
  if (3 != argc) {
    dmsf_usage();
    exit(1);
  }
  endian_init();
  memory_init(name, memory);
  n = dmsf(argv[1], argv[2], name);
  memory_dispose();
  if (0 == n) {
    return 0;
  } else if (255 == n) {
    return 255;
  } else {
    return 1;
  }
}
