/*
 * $Id: ztcv.c,v 1.6 2005/06/22 21:52:55 jon Exp $
 *
 * Calculate lift from tensor condensation representation
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include "endian.h"
#include "memory.h"
#include "parse.h"
#include "tcv.h"

static const char *name = "ztcv";

static void tcv_usage(void)
{
  fprintf(stderr, "%s: usage: %s %s <s> <file of left multiplicities> <file of right multiplicities> <input> <output> <basis 1> [<further bases>]\n", name, name, parse_usage());
}

int main(int argc, const char * const argv[])
{
  const char *mults_l, *mults_r, *in, *out;
  unsigned int s;
  int ret;

  argv = parse_line(argc, argv, &argc);
  if (7 > argc) {
    tcv_usage();
    exit(1);
  }
  endian_init();
  s = strtoul(argv[1], NULL, 0);
  if (0 >= s) {
    fprintf(stderr, "%s: no irreducible components, terminating\n", name);
    return 1;
  }
  mults_l = argv[2];
  mults_r = argv[3];
  in = argv[4];
  out = argv[5];
  memory_init(name, memory);
  ret = tco_lift(s, mults_l, mults_r, in, out, argc - 6, argv + 6, name);
  memory_dispose();
  if (1 == ret) {
    return 0;
  } else {
    return 1;
  }
}
