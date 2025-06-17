/*
 * $Id: zfn.c,v 1.1 2006/05/09 22:04:11 jon Exp $
 *
 * Filter list on nullity
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include "endian.h"
#include "fn.h"
#include "memory.h"
#include "parse.h"

static const char *name = "zfn";

static void fn_usage(void)
{
  fprintf(stderr, "%s: usage: %s %s <in_file> <out_file> <nullity> gen 1> [<gens>]\n", name, name, parse_usage());
}

int main(int argc, const char * const argv[])
{
  u32 nullity;
  int ret;

  argv = parse_line(argc, argv, &argc);
  if (5 > argc) {
    fn_usage();
    exit(1);
  }
  endian_init();
  memory_init(name, memory);
  nullity = strtoul(argv[3], NULL, 0);
  ret = filter_nullity(argv[1], argv[2], nullity, argc - 4, argv + 4, name);
  memory_dispose();
  return ret;
}
