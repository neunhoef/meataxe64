/*
 * $Id: zsumsf.c,v 1.3 2002/07/09 09:08:12 jon Exp $
 *
 * Compute sums in the group algebra in two matrices
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include "endian.h"
#include "memory.h"
#include "parse.h"
#include "sumsf.h"
#include "utils.h"

static const char *name = "zsumsf";

static unsigned int nullity = 0;

static void sumsf_usage(void)
{
  fprintf(stderr, "%s: usage: %s <in_file a> <in_file b> <out_file_stem> <tmp dir> <order a> <order b> <n> <nullity> [<memory>]\n", name, name);
}

static int acceptor(unsigned int rank, unsigned int nor, const char *file, const char *form)
{
  NOT_USED(file);
  NOT_USED(form);
  if (rank < nor && rank + nullity >= nor) {
    return 3;
  } else {
    return 0;
  }
}

int main(int argc, const char * const argv[])
{
  unsigned int memory = MEM_SIZE;
  unsigned int o_a, o_b, n;
  int res;

  argv = parse_line(argc, argv, &argc);
  if (9 != argc && 10 != argc) {
    sumsf_usage();
    exit(1);
  }
  o_a = strtoul(argv[5], NULL, 0);
  o_b = strtoul(argv[6], NULL, 0);
  n = strtoul(argv[7], NULL, 0);
  nullity = strtoul(argv[8], NULL, 0);
  if (10 == argc) {
    memory = strtoul(argv[9], NULL, 0);
  }
  if (0 == n) {
    fprintf(stderr, "%s: no sums requested\n", name);
    exit(1);
  }
  endian_init();
  memory_init(name, memory);
  res = sumsf(argv[1], argv[2], argv[3], argv[4], o_a, o_b, n, 0, name, &acceptor);
  if (0 != res) {
    printf("Failed to find a suitable element\n");
  }
  memory_dispose();
  return (0 != res);
}
