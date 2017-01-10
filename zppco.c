/*
 *
 * p Permutation condense
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include "endian.h"
#include "memory.h"
#include "parse.h"
#include "ppco.h"
#include "utils.h"

static const char *name = "zpco";

static void ppco_usage(void)
{
  fprintf(stderr, "%s: usage: %s %s <orbit_set> <perm> <condensation> <field order> <condensation group order>\n", name, name, parse_usage());
}

int main(int argc, const char * const argv[])
{
  u32 prime, group_order;
  const char *in_orb, *in_g, *out;
  endian_init();
  argv = parse_line(argc, argv, &argc);
  if (6 != argc) {
    ppco_usage();
    exit(1);
  }
  in_orb = argv[1];
  in_g = argv[2];
  out = argv[3];
  prime = strtoul(argv[4], NULL, 0);
  group_order = strtoul(argv[5], NULL, 0);
  memory_init(name, memory);
  endian_init();
  if (0 == ppcondense(in_orb, in_g, group_order, prime, out, name)) {
    exit(1);
  }
  memory_dispose();
  return 0;
}
