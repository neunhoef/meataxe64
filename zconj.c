/*
 * $Id: zconj.c,v 1.5 2004/01/31 13:24:51 jon Exp $
 *
 * Algebraic conjugate a matrix
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include "conj.h"
#include "endian.h"
#include "memory.h"
#include "parse.h"

static const char *name = "zconj";

static void conj_usage(void)
{
  fprintf(stderr, "%s: usage: %s %s <in_file> <out_file> <power>\n", name, name, parse_usage());
}

int main(int argc, const char * const argv[])
{
  const char *in;
  const char *out;
  unsigned int power;

  argv = parse_line(argc, argv, &argc);
  if (4 != argc) {
    conj_usage();
    exit(1);
  }
  in = argv[1];
  out = argv[2];
  power = strtoul(argv[3], NULL, 0);
  memory_init(name, memory);
  endian_init();
  if (0 == conjugate(in, out, power, name)) {
    exit(1);
  }
  memory_dispose();
  return 0;
}
