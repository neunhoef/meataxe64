/*
 * $Id
 *
 * Generate scaled identity matrix
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "endian.h"
#include "ident.h"
#include "memory.h"
#include "parse.h"

static const char *name = "zsid";

static void sid_usage(void)
{
  fprintf(stderr, "%s: usage: %s %s <field order> <nor> <noc> <elt> <out_file>\n", name, name, parse_usage());
}

int main(int argc, const char * const argv[])
{
  const char *out;
  unsigned int prime, noc, nor, elt;

  argv = parse_line(argc, argv, &argc);
  if (6 != argc) {
    sid_usage();
    exit(1);
  }
  out = argv[5];
  prime = strtoul(argv[1], NULL, 0);
  nor = strtoul(argv[2], NULL, 0);
  noc = strtoul(argv[3], NULL, 0);
  elt = strtoul(argv[4], NULL, 0);
  memory_init(name, 0);
  endian_init();
  if (0 == ident(prime, nor, noc, elt, out, name)) {
    exit(1);
  }
  memory_dispose();
  return 0;
}
