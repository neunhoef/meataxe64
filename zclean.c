/*
 * $Id: zclean.c,v 1.2 2002/10/14 19:11:51 jon Exp $
 *
 * Clean a set of vectors with another set
 *
 */

#include <stdio.h>
#include "clean_vectors.h"
#include "endian.h"
#include "memory.h"
#include "parse.h"

static const char *name = "zclean";

static void clean_usage(void)
{
  fprintf(stderr, "%s: usage: %s [-v] [-m <memory>] <clean vectors> <vectors to clean> <output>\n", name, name);
}

int main(int argc, const char * const argv[])
{
  int n;

  argv = parse_line(argc, argv, &argc);
  if (4 != argc) {
    clean_usage();
    exit(1);
  }
  endian_init();
  memory_init(name, memory);
  n = clean_vectors(argv[1], argv[2], argv[3], name);
  memory_dispose();
  if (1 == n) {
    return 0;
  } else {
    return 1;
  }
}
