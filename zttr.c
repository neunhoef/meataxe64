/*
 * $Id: zttr.c,v 1.2 2004/01/04 21:22:50 jon Exp $
 *
 * Transpose the order of a tensor product vector
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include "endian.h"
#include "memory.h"
#include "parse.h"
#include "ttr.h"

static const char *name = "zttr";

static void ttr_usage(void)
{
  fprintf(stderr, "%s: usage: %s %s <input columns> <in_file> <out_file>\n", name, name, parse_usage());
}

int main(int argc, const char * const argv[])
{
  const char *in;
  const char *out;
  unsigned int noc;

  argv = parse_line(argc, argv, &argc);
  if (4 != argc) {
    ttr_usage();
    exit(1);
  }
  in = argv[2];
  out = argv[3];
  memory_init(name, memory);
  endian_init();
  noc = strtoul(argv[1], NULL, 0);
  if (0 == ttr(noc, in, out, name)) {
    exit(1);
  }
  memory_dispose();
  return 0;
}
