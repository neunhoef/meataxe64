/*
 * $Id: ztco.c,v 1.3 2003/03/17 00:15:49 jon Exp $
 *
 * Tensor condense
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include "endian.h"
#include "memory.h"
#include "parse.h"
#include "tco.h"
#include "utils.h"

static const char *name = "ztco";

static void tco_usage(void)
{
  fprintf(stderr, "%s: usage: %s [-v] [-m <memory>] <s> <file of left multiplicities> <file of right multiplicities> <file of irreducible dimensions> <file of endomorphism dimensions> <left tensor> <right tensor> <output> <basis 1> <projection 1> [<further bases and projections>]\n", name, name);
}

int main(int argc, const char * const argv[])
{
  const char *mults_l, *mults_r, *irr, *end, *left, *right, *out;
  unsigned int s, ret;
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
  ret = tcondense(s, mults_l, mults_r, irr, end, left, right, out, argc - 9, argv + 9, name);
  memory_dispose();
  if (1 == ret) {
    return 0;
  } else if (255 == ret) {
    return 2;
  } else {
    return 1;
  }
}
