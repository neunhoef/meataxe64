/*
 * $Id: zclean.c,v 1.1 2002/10/12 14:17:06 jon Exp $
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
  fprintf(stderr, "%s: usage: %s <memory> <clean vectors> <vectors to clean> <output>\n", name, name);
}

int main(int argc, const char * const argv[])
{
  int n;
  unsigned int memory = MEM_SIZE;

  argv = parse_line(argc, argv, &argc);
  if (5 != argc) {
    clean_usage();
    exit(1);
  }
  endian_init();
  memory = strtoul(argv[1], NULL, 0);
  memory_init(name, memory);
  n = clean_vectors(argv[2], argv[3], argv[4], name);
  memory_dispose();
  if (1 == n) {
    return 0;
  } else {
    return 1;
  }
}
