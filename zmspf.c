/*
 * $Id: zmspf.c,v 1.3 2002/07/09 09:08:12 jon Exp $
 *
 * Spin some vectors under multiple generators, using intermediate files
 *
 */

#include <stdio.h>
#include "endian.h"
#include "memory.h"
#include "mspf.h"
#include "parse.h"

static const char *name = "zmspf";

static void mspf_usage(void)
{
  fprintf(stderr, "%s: usage: %s <in_file> <out_file> <tmp dir> <memory> <gen_1> [<gen>*]\n", name, name);
}

int main(int argc, const char * const argv[])
{
  unsigned int memory = MEM_SIZE;
  unsigned int dim;

  argv = parse_line(argc, argv, &argc);
  if (6 > argc) {
    mspf_usage();
    exit(1);
  }
  endian_init();
  memory = strtoul(argv[4], NULL, 0);
  memory_init(name, memory);
  dim = spin(argv[1], argv[2], argv[3], argc - 5, argv + 5, name);
  printf("%d\n", dim);
  memory_dispose();
  return 0;
}
