/*
 * $Id: zrranks.c,v 1.3 2002/07/09 09:08:12 jon Exp $
 *
 * Compute restricted sums in the group algebra in two matrices finding all of given nullity
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include "endian.h"
#include "memory.h"
#include "parse.h"
#include "sums.h"
#include "utils.h"

static const char *name = "zrranks";

static unsigned int nullity = 0;

static void rranks_usage(void)
{
  fprintf(stderr, "%s: usage: %s <in_file a> <in_file b> <out_file_stem> <order a> <order b> <n> <nullity> <subfield order> [<memory>]\n", name, name);
}

static int acceptor(unsigned int rank, unsigned int nor, const char *file, const char *form)
{
  NOT_USED(file);
  NOT_USED(form);
  if ((0 == nullity && rank == nor) || (rank < nor && rank + nullity >= nor)) {
    return 1;
  } else {
    return 0;
  }
}

int main(int argc, const char * const argv[])
{
  unsigned int memory = MEM_SIZE;
  unsigned int o_a, o_b, n, sub_order;
  int res;

  argv = parse_line(argc, argv, &argc);
  if (9 != argc && 10 != argc) {
    rranks_usage();
    exit(1);
  }
  o_a = strtoul(argv[4], NULL, 0);
  o_b = strtoul(argv[5], NULL, 0);
  n = strtoul(argv[6], NULL, 0);
  nullity = strtoul(argv[7], NULL, 0);
  sub_order = strtoul(argv[8], NULL, 0);
  if (10 == argc) {
    memory = strtoul(argv[9], NULL, 0);
  }
  if (0 == n) {
    fprintf(stderr, "%s: no ranks requested\n", name);
    exit(1);
  }
  endian_init();
  memory_init(name, memory);
  res = sums(argv[1], argv[2], argv[3], o_a, o_b, n, sub_order, name, &acceptor);
  memory_dispose();
  return 0;
}
