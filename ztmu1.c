/*
 * Tensor multiply a set of padded vectors by a pair of tensor
 * components
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

static const char *name = "ztmu1";

static void tmu1_usage(void)
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
    tmu1_usage();
    exit(1);
  }
  in1 = argv[1];
  in2 = argv[2];
  in3 = argv[3];
  out = argv[4];
  memory_init(name, memory);
  endian_init();
  if (0 == tmulp(in1, in2, in3, out, name)) {
    exit(1);
  }
  memory_dispose();
  return 0;
}
