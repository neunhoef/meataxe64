/*
 * $Id: zproj.c,v 1.3 2002/10/14 09:31:12 jon Exp $
 *
 * Project into quotient space representation
 *
 */

#include <stdio.h>
#include "endian.h"
#include "memory.h"
#include "parse.h"
#include "project.h"

static const char *name = "zproj";

static void proj_usage(void)
{
  fprintf(stderr, "%s: usage: %s [-v] [-m <memory>] <range> <input> <output>\n", name, name);
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
