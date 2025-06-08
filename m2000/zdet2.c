/*
 * Compute the determinant of a matrix
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include "endian.h"
#include "memory.h"
#include "det.h"
#include "parse.h"

static const char *name = "zdet2";

static void det_usage(void)
{
  fprintf(stderr, "%s: usage: %s %s <matrix>\n", name, name, parse_usage());
}

int main(int argc, const char * const argv[])
{
  word d;

  argv = parse_line(argc, argv, &argc);
  if (2 != argc) {
    det_usage();
    exit(1);
  }
  endian_init();
  memory_init(name, memory);
  if (0 == det2(argv[1], &d, name)) {
    exit(1);
  }
  printf("%" W_F "\n", d);
  memory_dispose();
  return 0;
}
