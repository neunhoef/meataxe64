/*
 * Compute the determinant of a matrix, using temporary files
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include "endian.h"
#include "memory.h"
#include "detf.h"
#include "parse.h" 
static const char *name = "zdetf";

static void detf_usage(void)
{
  fprintf(stderr, "%s: usage: %s %s <in_file> <temp dir>\n", name, name, parse_usage());
}

int main(int argc, const char * const argv[])
{
  word d;

  argv = parse_line(argc, argv, &argc);
  if (3 != argc) {
    detf_usage();
    exit(1);
  }
  endian_init();
  memory_init(name, memory);
  d = detf(argv[1], argv[2], name);
  printf("%" W_F "\n", d);
  memory_dispose();
  return 0;
}
