/*
 * $Id: zcfl.c,v 1.1 2006/05/09 22:04:11 jon Exp $
 *
 * Create filter list
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include "endian.h"
#include "cfl.h"
#include "memory.h"
#include "parse.h"

static const char *name = "zcfl";

static void cfl_usage(void)
{
  fprintf(stderr, "%s: usage: %s %s <prime power> <number of generators> <output>\n", name, name, parse_usage());
}

int main(int argc, const char * const argv[])
{
  u32 prime;
  u32 count;
  int ret;

  argv = parse_line(argc, argv, &argc);
  if (4 != argc) {
    cfl_usage();
    exit(1);
  }
  endian_init();
  memory_init(name, memory);
  prime = strtoul(argv[1], NULL, 0);
  count = strtoul(argv[2], NULL, 0);
  ret = create_filter_list(prime, count, argv[3], name);
  memory_dispose();
  return ret;
}
