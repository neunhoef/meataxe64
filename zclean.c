/*
 * $Id: zclean.c,v 1.3 2004/01/04 21:22:50 jon Exp $
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
  fprintf(stderr, "%s: usage: %s %s <clean vectors> <vectors to clean> <output>\n", name, name, parse_usage());
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
