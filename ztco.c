/*
 * $Id: ztco.c,v 1.1 2002/08/27 17:12:38 jon Exp $
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
  fprintf(stderr, "%s: usage: %s <s> <file of left multiplicities> <file of right multiplicities> <file of irreducible dimensions> <file of endomorphism dimensions> <memory> <left tensor> <right tensor> <output> <basis 1> <projection 1> [<further bases and projections>]\n", name, name);
}

int main(int argc, const char * const argv[])
{
  const char *mults_l, *mults_r, *irr, *end, *left, *right, *out;
  unsigned int memory = MEM_SIZE, s, ret;
  endian_init();
  argv = parse_line(argc, argv, &argc);
  if (12 > argc || 0 != (argc % 2)) {
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
  memory = strtoul(argv[6], NULL, 0);
  left = argv[7];
  right = argv[8];
  out = argv[9];
  memory_init(name, memory);
  endian_init();
  ret = tcondense(s, mults_l, mults_r, irr, end, left, right, out, argc - 10, argv + 10, name);
  memory_dispose();
  if (1 == ret) {
    return 0;
  } else if (255 == ret) {
    return 2;
  } else {
    return 1;
  }
}
