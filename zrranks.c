/*
 * $Id: zrranks.c,v 1.5 2002/09/11 15:34:59 jon Exp $
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
  fprintf(stderr, "%s: usage: %s <out_file_stem> <n> <nullity> <subfield order> <memory> <in_file a> <order a> <in_file b> <order b>\n", name, name);
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
  unsigned int n, sub_order;
  int res;

  argv = parse_line(argc, argv, &argc);
  if (10 > argc || 0 != argc % 2) {
    rranks_usage();
    exit(1);
  }
  n = strtoul(argv[2], NULL, 0);
  nullity = strtoul(argv[3], NULL, 0);
  sub_order = strtoul(argv[4], NULL, 0);
  memory = strtoul(argv[5], NULL, 0);
  if (0 == n) {
    fprintf(stderr, "%s: no ranks requested\n", name);
    exit(1);
  }
  endian_init();
  memory_init(name, memory);
  res = sums(argv[1], n, argc - 6, argv + 6, sub_order, &acceptor, name);
  memory_dispose();
  return 0;
}
