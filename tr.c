/*
 * $Id: tr.c,v 1.4 2002/03/07 13:43:30 jon Exp $
 *
 * Transpose a matrix
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include "endian.h"
#include "memory.h"
#include "tra.h"

static const char *name = "ztr";

static void tr_usage(void)
{
  fprintf(stderr, "%s: usage: %s <in_file> <out_file> [<memory>]\n", name, name);
}

int main(int argc, const char * const argv[])
{
  const char *in;
  const char *out;
  unsigned int memory = MEM_SIZE;

  if (3 != argc && 4 != argc) {
    tr_usage();
    exit(1);
  }
  in = argv[1];
  out = argv[2];
  if (4 == argc) {
    memory = strtoul(argv[3], NULL, 0);
  }
  memory_init(name, memory);
  endian_init();
  if (0 == tra(in, out, name)) {
    exit(1);
  }
  memory_dispose();
  return 0;
}
