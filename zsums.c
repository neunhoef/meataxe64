/*
 * $Id: zsums.c,v 1.16 2005/06/22 21:52:55 jon Exp $
 *
 * Compute sums in the group algebra in two matrices
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include "endian.h"
#include "memory.h"
#include "parse.h"
#include "sums.h"
#include "utils.h"

static const char *name = "zsums";

static u32 nullity = 0;

static void sums_usage(void)
{
  fprintf(stderr, "%s: usage: %s %s <out_file_stem> <n> <nullity> <in_file a> <order a> <in_file b> <order b>\n", name, name, parse_usage());
}

static int acceptor(u32 rank, u32 nor, const char *file, const char *form)
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
  u32 n;
  int res;

  argv = parse_line(argc, argv, &argc);
  if (8 > argc || 0 != argc % 2) {
    sums_usage();
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
  res = sums(argv[1], n, argc - 4, argv + 4, 0, &acceptor, 1, 0, name);
  if (255 == res) {
    printf("Failed to find a suitable element\n");
  }
  memory_dispose();
  return res;
}
