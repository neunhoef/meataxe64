/*
 * $Id: zsums.c,v 1.7 2002/07/03 12:06:54 jon Exp $
 *
 * Compute sums in the group algebra in two matrices
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include "endian.h"
#include "memory.h"
#include "sums.h"
#include "utils.h"

static const char *name = "zsums";

static unsigned int nullity = 0;

static void sums_usage(void)
{
  fprintf(stderr, "%s: usage: %s <in_file a> <in_file b> <out_file_stem> <order a> <order b> <n> <nullity> [<memory>]\n", name, name);
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

  if (8 != argc && 9 != argc) {
    sums_usage();
    exit(1);
  }
  o_a = strtoul(argv[4], NULL, 0);
  o_b = strtoul(argv[5], NULL, 0);
  n = strtoul(argv[6], NULL, 0);
  nullity = strtoul(argv[7], NULL, 0);
  if (9 == argc) {
    memory = strtoul(argv[8], NULL, 0);
  }
  if (0 == n) {
    fprintf(stderr, "%s: no sums requested\n", name);
    exit(1);
  }
  endian_init();
  memory_init(name, memory);
  res = sums(argv[1], argv[2], argv[3], o_a, o_b, n, 0, name, &acceptor);
  if (0 != res) {
    printf("Failed to find a suitable element\n");
  }
  memory_dispose();
  return (0 != res);
}
