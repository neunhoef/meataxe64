/*
 * $Id: zsymsq.c,v 1.1 2002/02/12 23:10:24 jon Exp $
 *
 * Symmetric square a matrix
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "endian.h"
#include "memory.h"
#include "utils.h"
#include "powers.h"

static const char *name = "zsymsq";

static void symsq_usage(void)
{
  fprintf(stderr, "%s: usage: %s <in_file> <out_file> [<memory>]\n", name, name);
}

int main(int argc, const char * const argv[])
{
  const char *in;
  const char *out;
  unsigned int memory = MEM_SIZE;

  if (3 != argc && 4 != argc) {
    symsq_usage();
    exit(1);
  }
  in = argv[1];
  out = argv[2];
  if (4 == argc) {
    memory = strtoul(argv[3], NULL, 0);
  }
  endian_init();
  memory_init(name, memory);
  if (0 == sym_square(in, out, name)) {
    exit(1);
  }
  memory_dispose();
  return 0;
}
