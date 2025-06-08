/*
 * $Id: zdc.c,v 1.1 2005/06/22 21:52:54 jon Exp $
 *
 * Calculate direct complement
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include "endian.h"
#include "memory.h"
#include "parse.h"
#include "dc.h"

static const char *name = "zdc";

static void dc_usage(void)
{
  fprintf(stderr, "%s: usage: %s %s <range> <output>\n", name, name, parse_usage());
}

int main(int argc, const char * const argv[])
{
  argv = parse_line(argc, argv, &argc);
  if (3 != argc) {
    dc_usage();
    exit(1);
  }
  endian_init();
  memory_init(name, memory);
  direct_complement(argv[1], argv[2], name);
  memory_dispose();
  return 0;
}
