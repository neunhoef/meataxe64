/*
 * $Id: ztmu.c,v 1.2 2002/07/09 09:08:12 jon Exp $
 *
 * Tensor multiply three matrices to give a fourth
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "endian.h"
#include "memory.h"
#include "parse.h"
#include "tmul.h"
#include "utils.h"

static const char *name = "ztmu";

static void tmu_usage(void)
{
  fprintf(stderr, "%s: usage: %s <in_file> <left tensor> <right tensor> <out_file> [<memory>]\n", name, name);
}

int main(int argc, const char * const argv[])
{
  const char *in1;
  const char *in2;
  const char *in3;
  const char *out;
  unsigned int memory = MEM_SIZE;

  argv = parse_line(argc, argv, &argc);
  if (5 != argc && 6 != argc) {
    tmu_usage();
    exit(1);
  }
  in1 = argv[1];
  in2 = argv[2];
  in3 = argv[3];
  out = argv[4];
  if (6 == argc) {
    memory = strtoul(argv[5], NULL, 0);
  }
  memory_init(name, memory);
  endian_init();
  if (0 == tmul(in1, in2, in3, out, name)) {
    exit(1);
  }
  memory_dispose();
  return 0;
}
