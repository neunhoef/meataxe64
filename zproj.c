/*
 * $Id: zproj.c,v 1.5 2004/08/21 13:22:31 jon Exp $
 *
 * Project into quotient space representation
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include "endian.h"
#include "memory.h"
#include "parse.h"
#include "project.h"

static const char *name = "zproj";

static void proj_usage(void)
{
  fprintf(stderr, "%s: usage: %s %s <range> <input> <output>\n", name, name, parse_usage());
}

int main(int argc, const char * const argv[])
{
  argv = parse_line(argc, argv, &argc);
  if (4 != argc) {
    proj_usage();
    exit(1);
  }
  endian_init();
  memory_init(name, memory);
  project(argv[1], argv[2], argv[3], name);
  memory_dispose();
  return 0;
}
