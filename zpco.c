/*
 * $Id: zpco.c,v 1.2 2002/07/09 09:08:12 jon Exp $
 *
 * Permutation condense
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include "endian.h"
#include "memory.h"
#include "parse.h"
#include "pco.h"
#include "utils.h"

static const char *name = "zpco";

static void pco_usage(void)
{
  fprintf(stderr, "%s: usage: %s <orbit_set> <perm> <condensation> <field order> [<memory>]\n", name, name);
}

int main(int argc, const char * const argv[])
{
  const char *in1, *in2, *out;
  unsigned int memory = MEM_SIZE, prime;
  endian_init();
  argv = parse_line(argc, argv, &argc);
  if (5 != argc && 6 != argc) {
    pco_usage();
    exit(1);
  }
  in1 = argv[1];
  in2 = argv[2];
  out = argv[3];
  prime = strtoul(argv[4], NULL, 0);
  if (6 == argc) {
    memory = strtoul(argv[5], NULL, 0);
  }
  memory_init(name, memory);
  endian_init();
  if (0 == pcondense(in1, in2, prime, out, name)) {
    exit(1);
  }
  memory_dispose();
  return 0;
}
