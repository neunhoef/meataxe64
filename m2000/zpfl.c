/*
 * $Id: zpfl.c,v 1.1 2006/05/09 22:13:56 jon Exp $
 *
 * Print filter list
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include "endian.h"
#include "pfl.h"
#include "memory.h"
#include "parse.h"

static const char *name = "zpfl";

static void pfl_usage(void)
{
  fprintf(stderr, "%s: usage: %s %s <in_file> [<gens>]\n", name, name, parse_usage());
}

int main(int argc, const char * const argv[])
{
  argv = parse_line(argc, argv, &argc);
  if (3 > argc) {
    pfl_usage();
    exit(1);
  }
  endian_init();
  memory_init(name, memory);
  print_filter(argv[1], argc - 2, argv + 2, name);
  memory_dispose();
  return 0;
}
