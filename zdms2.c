/*
 * Compute the determinant of a matrix mod field squares in characteristic 2
 * Returns 0 for is a square, 255 for is not a square
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include "endian.h"
#include "memory.h"
#include "dms.h"
#include "parse.h"

static const char *name = "zdms2";

static void dms_usage(void)
{
  fprintf(stderr, "%s: usage: %s %s <matrix>\n", name, name, parse_usage());
}

int main(int argc, const char * const argv[])
{
  int n = 0;

  argv = parse_line(argc, argv, &argc);
  if (2 != argc) {
    dms_usage();
    exit(1);
  }
  endian_init();
  memory_init(name, memory);
  /*n = dms(argv[1], name);*/
  memory_dispose();
  if (0 == n) {
    return 0;
  } else if (255 == n) {
    return 255;
  } else {
    return 1;
  }
}
