/*
 * $Id: znu0.c,v 1.1 2021/08/01 09:53:56 jon Exp $
 *
 * Filter list of group algebra elements retaining
 * only those of full rank (nullity 0)
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include "endian.h"
#include "memory.h"
#include "parse.h"
#include "script.h"
#include "sums.h"
#include "utils.h"

static const char *name = "znu0";

static void nu0_usage(void)
{
  fprintf(stderr, "%s: usage: %s %s <scripts> <in_file a> [<other elements>]\n", name, name, parse_usage());
}

int main(int argc, const char * const argv[])
{
  int res = 0;

  argv = parse_line(argc, argv, &argc);
  if (2 > argc) {
    nu0_usage();
    exit(1);
  }
  endian_init();
  memory_init(name, memory);
  res = exec_scripts(argv[1], argc - 2, argv + 2, name);
  memory_dispose();
  return res;
}
