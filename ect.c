/*
 * $Id: ect.c,v 1.4 2002/10/14 19:11:51 jon Exp $
 *
 * Count the non-zero elements in an exploded matrix
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include "count.h"
#include "endian.h"
#include "files.h"
#include "map.h"
#include "memory.h"
#include "parse.h"

static const char *name = "ect";

static void ect_usage(void)
{
  fprintf(stderr, "%s: usage: %s [-v] [-m <memory>] <in_dir>\n", name, name);
}

int main(int argc, const char *const argv[])
{
  unsigned int total = 0, ototal, htotal = 0;
  unsigned int col_pieces, row_pieces;
  unsigned int i, j;
  const char **names;
  argv = parse_line(argc, argv, &argc);
  if (2 != argc) {
    ect_usage();
    exit(1);
  }
  input_map(name, argv[1], &col_pieces, &row_pieces, &names);
  endian_init();
  memory_init(name, memory);
  for (i = 0; i < row_pieces; i++) {
    for (j = 0; j < col_pieces; j++) {
      ototal = total;
      total += count(pathname(argv[1], names[i * col_pieces + j]), name);
      if (total < ototal) {
        /* Wrapped around */
        htotal += 1;
      }
    }
  }
  if (0 != htotal) {
    printf("0x%x%8x\n", htotal, total);
  } else {
    printf("%u\n", total);
  }
  return 0;
}
