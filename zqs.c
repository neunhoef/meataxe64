/*
 * $Id: zqs.c,v 1.2 2002/07/09 09:08:12 jon Exp $: zss.c,v 1.1 2001/11/25 00:17:19 jon Exp $
 *
 * Calculate quotient space representation
 *
 */

#include <stdio.h>
#include "endian.h"
#include "memory.h"
#include "parse.h"
#include "qs.h"

static const char *name = "zqs";

static void qs_usage(void)
{
  fprintf(stderr, "%s: usage: %s <range> <generator> <output> [<memory>]\n", name, name);
}

int main(int argc, const char * const argv[])
{
  unsigned int memory = MEM_SIZE;

  argv = parse_line(argc, argv, &argc);
  if (4 != argc && 5 != argc) {
    qs_usage();
    exit(1);
  }
  endian_init();
  if (5 == argc) {
    memory = strtoul(argv[4], NULL, 0);
  }
  memory_init(name, memory);
  quotient(argv[1], argv[2], argv[3], name);
  memory_dispose();
  return 0;
}
