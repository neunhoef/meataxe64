/*
 * $Id: zte.c,v 1.1 2001/12/01 10:46:02 jon Exp $
 *
 * Tensor two matrices to give a third
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "memory.h"
#include "utils.h"
#include "te.h"

static const char *name = "zte";

static void te_usage(void)
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
    te_usage();
    exit(1);
  }
  in1 = argv[1];
  in2 = argv[2];
  out = argv[3];
  if (5 == argc) {
    memory = strtoul(argv[4], NULL, 0);
  }
  memory_init(name, memory);
  if (0 == tensor(in1, in2, out, name)) {
    exit(1);
  }
  memory_dispose();
  return 0;
}
