/*
 * $Id: zrsumsf.c,v 1.5 2003/08/04 20:41:57 jon Exp $
 *
 * Compute restricted sums in the group algebra in two matrices
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include "endian.h"
#include "memory.h"
#include "parse.h"
#include "sumsf.h"
#include "utils.h"

static const char *name = "zrsumsf";

static unsigned int nullity = 0;

static void rsumsf_usage(void)
{
  fprintf(stderr, "%s: usage: %s [-v] [-m <memory>] <out_file_stem> <n> <nullity> <subfield order> <tmp dir> <in_file a> <order a> <in_file b> <order b>\n", name, name);
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
  unsigned int n, sub_order;
  int res;

  argv = parse_line(argc, argv, &argc);
  if (10 > argc || 0 != argc % 2) {
    rsumsf_usage();
    exit(1);
  }
  n = strtoul(argv[2], NULL, 0);
  nullity = strtoul(argv[3], NULL, 0);
  sub_order = strtoul(argv[4], NULL, 0);
  if (0 == n) {
    fprintf(stderr, "%s: no sums requested\n", name);
    exit(1);
  }
  endian_init();
  memory_init(name, memory);
  res = sumsf(argv[1], argv[5], n, argc - 6, argv + 6, sub_order, &acceptor, 0, name);
  if (255 == res) {
    printf("Failed to find a suitable element\n");
  }
  memory_dispose();
  return res;
}
