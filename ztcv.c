/*
 * $Id: ztcv.c,v 1.1 2002/08/27 17:12:38 jon Exp $
 *
 * Calculate lift from tensor condensation representation
 *
 */

#include <stdio.h>
#include "endian.h"
#include "memory.h"
#include "parse.h"
#include "tcv.h"

static const char *name = "ztcv";

static void tcv_usage(void)
{
  fprintf(stderr, "%s: usage: %s <s> <file of left multiplicities> <file of right multiplicities> <memory> <input> <output> <basis 1> [<further bases>]\n", name, name);
}

int main(int argc, const char * const argv[])
{
  unsigned int memory = MEM_SIZE, s, ret;
  const char *mults_l, *mults_r, *in, *out;

  argv = parse_line(argc, argv, &argc);
  if (8 > argc) {
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
  memory = strtoul(argv[4], NULL, 0);
  in = argv[5];
  out = argv[6];
  memory_init(name, memory);
  ret = lift(s, mults_l, mults_r, in, out, argc - 7, argv + 7, name);
  memory_dispose();
  if (1 == ret) {
    return 0;
  } else {
    return 1;
  }
}
