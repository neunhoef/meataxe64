/*
 * $Id: zsums.c,v 1.10 2002/09/11 15:34:59 jon Exp $
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

static unsigned int nullity = 0;

static void sums_usage(void)
{
  fprintf(stderr, "%s: usage: %s <out_file_stem> <n> <nullity> <memory> <in_file a> <order a> <in_file b> <order b>\n", name, name);
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
  unsigned int n, memory = MEM_SIZE;
  int res;

  argv = parse_line(argc, argv, &argc);
  if (9 > argc || 1 != argc % 2) {
    sums_usage();
    exit(1);
  }
  n = strtoul(argv[2], NULL, 0);
  nullity = strtoul(argv[3], NULL, 0);
  memory = strtoul(argv[4], NULL, 0);
  if (0 == n) {
    fprintf(stderr, "%s: no sums requested\n", name);
    exit(1);
  }
  endian_init();
  memory_init(name, memory);
  res = sums(argv[1], n, argc - 5, argv + 5, 0, &acceptor, name);
  if (0 != res) {
    printf("Failed to find a suitable element\n");
  }
  memory_dispose();
  return (0 != res);
}
