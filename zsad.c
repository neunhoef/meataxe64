/*
 * $Id: zsad.c,v 1.5 2004/01/31 13:24:51 jon Exp $
 *
 * Add a matrix and a scaled matrix to give a third
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include "add.h"
#include "endian.h"
#include "memory.h"
#include "parse.h"

static const char *name = "zsad";

static void sad_usage(void)
{
  fprintf(stderr, "%s: usage: %s %s <in_file> <in_file> <out_file> <scalar>\n", name, name, parse_usage());
}

/* lambda a + b */
int main(int argc, const char * const argv[])
{
  const char *in1;
  const char *in2;
  const char *out;
  unsigned int scalar;

  argv = parse_line(argc, argv, &argc);
  if (5 != argc) {
    sad_usage();
    exit(1);
  }
  in1 = argv[1];
  in2 = argv[2];
  out = argv[3];
  scalar = strtoul(argv[4], NULL, 0);
  memory_init(name, memory);
  endian_init();
  if (0 == scaled_add(in1, in2, out, scalar, name)) {
    exit(1);
  }
  memory_dispose();
  return 0;
}
