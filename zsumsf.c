/*
 * $Id: zsumsf.c,v 1.10 2003/08/10 14:30:25 jon Exp $
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
  fprintf(stderr, "%s: usage: %s [-v] <-m <memory>] <out_file_stem> <n> <nullity> <tmp dir> <in_file a> <order a> <in_file b> <order b>\n", name, name);
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
  unsigned int n;
  int res;

  argv = parse_line(argc, argv, &argc);
  if (9 > argc || 1 != argc % 2) {
    sumsf_usage();
    exit(1);
  }
  n = strtoul(argv[2], NULL, 0);
  nullity = strtoul(argv[3], NULL, 0);
  if (0 == n) {
    fprintf(stderr, "%s: no sums requested\n", name);
    exit(1);
  }
  endian_init();
  memory_init(name, memory);
  res = sumsf(argv[1], argv[4], n, argc - 5, argv + 5, 0, &acceptor, 1, 0, name);
  if (255 == res) {
    printf("Failed to find a suitable element\n");
  }
  memory_dispose();
  return res;
}
