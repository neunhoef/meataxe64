/*
 * $Id: zrand.c,v 1.1 2002/07/20 12:55:21 jon Exp $
 *
 * Generate random matrix
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "endian.h"
#include "memory.h"
#include "parse.h"
#include "rand.h"

static const char *name = "zrand";

static void rand_usage(void)
{
  fprintf(stderr, "%s: usage: %s <field order> <nor> <noc> <out_file>\n", name, name);
}

int main(int argc, const char * const argv[])
{
  const char *out;
  unsigned int prime, noc, nor;

  argv = parse_line(argc, argv, &argc);
  if (5 != argc) {
    rand_usage();
    exit(1);
  }
  out = argv[4];
  prime = strtoul(argv[1], NULL, 0);
  nor = strtoul(argv[2], NULL, 0);
  noc = strtoul(argv[3], NULL, 0);
  memory_init(name, 0);
  endian_init();
  if (0 == random(prime, nor, noc, out, name)) {
    exit(1);
  }
  memory_dispose();
  return 0;
}
