/*
 * $Id: zqs.c,v 1.5 2004/08/21 13:22:31 jon Exp $: zss.c,v 1.1 2001/11/25 00:17:19 jon Exp $
 *
 * Calculate quotient space representation
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include "endian.h"
#include "memory.h"
#include "parse.h"
#include "qs.h"

static const char *name = "zqs";

static void qs_usage(void)
{
  fprintf(stderr, "%s: usage: %s %s <range> <generator> <output>\n", name, name, parse_usage());
}

int main(int argc, const char * const argv[])
{
  argv = parse_line(argc, argv, &argc);
  if (4 != argc) {
    qs_usage();
    exit(1);
  }
  endian_init();
  memory_init(name, memory);
  quotient(argv[1], argv[2], argv[3], name);
  memory_dispose();
  return 0;
}
