/*
 * $Id: zsbf.c,v 1.3 2002/07/09 09:08:12 jon Exp $
 *
 * Spin some vectors under two generators to obtain a standard base
 * using intermediate files in a temporary directory
 *
 */

#include <stdio.h>
#include "endian.h"
#include "memory.h"
#include "parse.h"
#include "sbf.h"

static const char *name = "zsbf";

static void sbf_usage(void)
{
  fprintf(stderr, "%s: usage: %s <in_file> <out_file> <gen_a> <gen_b> <tmp dir> [<memory>]\n", name, name);
}

int main(int argc, const char * const argv[])
{
  unsigned int memory = MEM_SIZE;
  unsigned int dim;

  argv = parse_line(argc, argv, &argc);
  if (6 != argc && 7 != argc) {
    sbf_usage();
    exit(1);
  }
  endian_init();
  if (7 == argc) {
    memory = strtoul(argv[6], NULL, 0);
  }
  memory_init(name, memory);
  dim = spin(argv[1], argv[2], argv[3], argv[4], argv[5], name);
  printf("%d\n", dim);
  memory_dispose();
  return 0;
}
