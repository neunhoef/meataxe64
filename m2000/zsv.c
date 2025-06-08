/*
 * $Id: zsv.c,v 1.1 2006/03/04 09:02:06 jon Exp $
 *
 * Compute a Schreier vector and back vector
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include "endian.h"
#include "memory.h"
#include "parse.h"
#include "sv.h"
#include "utils.h"

static const char *name = "zsv";

static void sv_usage(void)
{
  fprintf(stderr, "%s: usage: %s %s <orbit point> <schreier vector> <back vector> <gen_1> [<gen>*]\n", name, name, parse_usage());
}

int main(int argc, const char * const argv[])
{
  u32 point;
  int res;

  argv = parse_line(argc, argv, &argc);
  if (5 > argc) {
    sv_usage();
    exit(1);
  }
  point = strtoul(argv[1], NULL, 0);
  endian_init();
  memory_init(name, memory);
  res = sv(point, argv[2], argv[3], argc - 4, argv + 4, name);
  memory_dispose();
  return res;
}
