/*
 * $Id: zns.c,v 1.2 2002/07/09 09:08:12 jon Exp $
 *
 * Compute the null space of a matrix
 *
 */

#include <stdio.h>
#include "endian.h"
#include "memory.h"
#include "ns.h"
#include "parse.h"

static const char *name = "zns";

static void ns_usage(void)
{
  fprintf(stderr, "%s: usage: %s <in_file> <out_file> [<memory>]\n", name, name);
}

int main(int argc, const char * const argv[])
{
  unsigned int n;
  unsigned int memory = MEM_SIZE;

  argv = parse_line(argc, argv, &argc);
  if (3 != argc && 4 != argc) {
    ns_usage();
    exit(1);
  }
  endian_init();
  if (4 == argc) {
    memory = strtoul(argv[3], NULL, 0);
  }
  memory_init(name, memory);
  n = nullspace(argv[1], argv[2], name);
  printf("%d\n", n);
  memory_dispose();
  return 0;
}
