/*
 * $Id: mu.c,v 1.5 2001/09/18 23:15:46 jon Exp $
 *
 * Multiply two matrices to give a third
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "memory.h"
#include "utils.h"
#include "mul.h"

static const char *name = "mu";

static void mu_usage(void)
{
  fprintf(stderr, "%s: usage: %s <in_file> <in_file> <out_file> [<memory>]\n", name, name);
}

int main(int argc, const char * const argv[])
{
  const char *in1;
  const char *in2;
  const char *out;
  unsigned int memory = MEM_SIZE;

  if (4 != argc && 5 != argc) {
    mu_usage();
    exit(1);
  }
  in1 = argv[1];
  in2 = argv[2];
  out = argv[3];
  if (5 == argc) {
    if (0 == read_decimal(argv[4], strlen(argv[4]), &memory)) {
      fprintf(stderr, "%s: failed to read memory size from command line\n", name);
      return 0;
    }
  }
  memory_init(name, memory);
  if (0 == mul(in1, in2, out, "mu")) {
    exit(1);
  }
  memory_dispose();
  return 0;
}
