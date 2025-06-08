/*
 * $Id: ztmu.c,v 1.4 2004/01/04 21:22:50 jon Exp $
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
  fprintf(stderr, "%s: usage: %s %s <in_file> <left tensor> <right tensor> <out_file>\n", name, name, parse_usage());
}

int main(int argc, const char * const argv[])
{
  const char *in1;
  const char *in2;
  const char *in3;
  const char *out;

  argv = parse_line(argc, argv, &argc);
  if (5 != argc) {
    tmu_usage();
    exit(1);
  }
  in1 = argv[1];
  in2 = argv[2];
  in3 = argv[3];
  out = argv[4];
  memory_init(name, memory);
  endian_init();
  if (0 == tmul(in1, in2, in3, out, name)) {
    exit(1);
  }
  memory_dispose();
  return 0;
}
