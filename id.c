/*
 * $Id: id.c,v 1.14 2004/01/04 21:22:50 jon Exp $: ad.c,v 1.1 2001/08/30 18:31:45 jon Exp $
 *
 * Generate identity matrix
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "endian.h"
#include "ident.h"
#include "memory.h"
#include "parse.h"

static const char *name = "zid";

static void id_usage(void)
{
  fprintf(stderr, "%s: usage: %s %s <field order> <nor> <noc> <out_file>\n", name, name, parse_usage());
}

int main(int argc, const char * const argv[])
{
  const char *out;
  unsigned int prime, noc, nor;

  argv = parse_line(argc, argv, &argc);
  if (5 != argc) {
    id_usage();
    exit(1);
  }
  out = argv[4];
  prime = strtoul(argv[1], NULL, 0);
  nor = strtoul(argv[2], NULL, 0);
  noc = strtoul(argv[3], NULL, 0);
  memory_init(name, 0);
  endian_init();
  if (0 == ident(prime, nor, noc, 1, out, name)) {
    exit(1);
  }
  memory_dispose();
  return 0;
}
