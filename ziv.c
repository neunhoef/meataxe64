/*
 * $Id: ziv.c,v 1.2 2002/07/09 09:08:12 jon Exp $
 *
 * Invert a matrix
 *
 */

#include <stdio.h>
#include "endian.h"
#include "iv.h"
#include "memory.h"
#include "parse.h"

static const char *name = "ziv";

static void iv_usage(void)
{
  fprintf(stderr, "%s: usage: %s <in_file> <out_file> [<memory>]\n", name, name);
}

int main(int argc, const char * const argv[])
{
  unsigned int memory = MEM_SIZE;

  argv = parse_line(argc, argv, &argc);
  if (3 != argc && 4 != argc) {
    iv_usage();
    exit(1);
  }
  endian_init();
  if (4 == argc) {
    memory = strtoul(argv[3], NULL, 0);
  }
  memory_init(name, memory);
  invert(argv[1], argv[2], name);
  memory_dispose();
  return 0;
}
