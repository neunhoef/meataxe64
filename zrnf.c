/*
 * $Id: zrnf.c,v 1.4 2002/07/09 09:08:12 jon Exp $
 *
 * Compute the rank of a matrix, using temporary files
 *
 */

#include <stdio.h>
#include "endian.h"
#include "memory.h"
#include "parse.h"
#include "rnf.h"

static const char *name = "zrnf";

static void rnf_usage(void)
{
  fprintf(stderr, "%s: usage: %s <in_file> <temp dir> [<memory>]\n", name, name);
}

int main(int argc, const char * const argv[])
{
  unsigned int n;
  unsigned int memory = MEM_SIZE;

  argv = parse_line(argc, argv, &argc);
  if (3 != argc && 4 != argc) {
    rnf_usage();
    exit(1);
  }
  endian_init();
  if (4 == argc) {
    memory = strtoul(argv[3], NULL, 0);
  }
  memory_init(name, memory);
  n = rank(argv[1], argv[2], NULL, 0, 0, name);
  printf("%d\n", n);
  memory_dispose();
  return 0;
}
