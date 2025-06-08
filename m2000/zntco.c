/*
 * $Id: zntco.c,v 1.1 2005/12/17 14:43:55 jon Exp $
 *
 * Tensor condense (new algorithm)
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include "endian.h"
#include "memory.h"
#include "parse.h"
#include "newtco.h"
#include "tco.h"
#include "utils.h"

static const char *name = "zntco";

static void tco_usage(void)
{
  fprintf(stderr, "%s: usage: %s %s <s> <file of left multiplicities> <file of right multiplicities> <file of irreducible dimensions> <file of endomorphism dimensions> <left tensor> <right tensor> <output> <basis 1> <projection 1> [<further bases and projections>]\n", name, name, parse_usage());
}

int main(int argc, const char * const argv[])
{
  const char *mults_l, *mults_r, *irr, *end, *left, *right, *out;
  u32 s, ret;
  endian_init();
  argv = parse_line(argc, argv, &argc);
  if (11 > argc || 1 != (argc % 2)) {
    tco_usage();
    exit(1);
  }
  s = strtoul(argv[1], NULL, 0);
  if (0 >= s) {
    fprintf(stderr, "%s: no irreducible components, terminating\n", name);
    return 1;
  }
  mults_l = argv[2];
  mults_r = argv[3];
  irr = argv[4];
  end = argv[5];
  left = argv[6];
  right = argv[7];
  out = argv[8];
  memory_init(name, memory);
  endian_init();
  ret = tcondense(s, mults_l, mults_r, irr, end, left, right, out, NULL, 0, 0, argc - 9, argv + 9, name);
  memory_dispose();
  if (1 == ret) {
    return 0;
  } else if (255 == ret) {
    return 2;
  } else {
    return 1;
  }
}
