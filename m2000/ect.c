/*
 * $Id: ect.c,v 1.6 2005/06/22 21:52:53 jon Exp $
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
  fprintf(stderr, "%s: usage: %s %s <in_dir>\n", name, name, parse_usage());
}

int main(int argc, const char *const argv[])
{
  u32 total = 0, ototal, htotal = 0;
  u32 col_pieces, row_pieces;
  u32 i, j;
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
