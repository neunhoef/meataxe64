/*
 * $Id: zskfi.c,v 1.2 2002/07/09 09:08:12 jon Exp $
 *
 * Skew fifth a matrix
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "endian.h"
#include "memory.h"
#include "parse.h"
#include "powers.h"
#include "utils.h"

static const char *name = "zskfi";

static void skfi_usage(void)
{
  fprintf(stderr, "%s: usage: %s <in_file> <out_file> [<memory>]\n", name, name);
}

int main(int argc, const char * const argv[])
{
  const char *in;
  const char *out;
  unsigned int memory = MEM_SIZE;

  argv = parse_line(argc, argv, &argc);
  if (3 != argc && 4 != argc) {
    skfi_usage();
    exit(1);
  }
  in = argv[1];
  out = argv[2];
  if (4 == argc) {
    memory = strtoul(argv[3], NULL, 0);
  }
  endian_init();
  memory_init(name, memory);
  if (0 == skew_fifth(in, out, name)) {
    exit(1);
  }
  memory_dispose();
  return 0;
}
