/*
 * $Id: ead.c,v 1.8 2004/01/04 21:22:50 jon Exp $
 *
 * Exploded add
 *
 * Three arguments
 * 1) First addend dir
 * 2) Second addend dir
 * 3) Result dir
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "add.h"
#include "endian.h"
#include "files.h"
#include "map.h"
#include "memory.h"
#include "parse.h"
#include "utils.h"

static const char *name = "ead";

static void ead_usage(void)
{
  fprintf(stderr, "%s: usage: %s %s <dir1> <dir2> <dir3>\n", name, name, parse_usage());
}

int main(int argc,  const char *const argv[])
{
  unsigned int col_pieces1, row_pieces1;
  unsigned int col_pieces2, row_pieces2;
  unsigned int i, j;
  const char **names1, **names2, **names3;
  argv = parse_line(argc, argv, &argc);
  memory_init(name, 0);
  endian_init();
  /******  First check the number of input arguments  */
  if (argc != 4) {
    ead_usage();
    exit(1);
  }
  /* Now get a look at the map file */
  input_map(name, argv[1], &col_pieces1, &row_pieces1, &names1);
  input_map(name, argv[2], &col_pieces2, &row_pieces2, &names2);
  if (row_pieces1 != row_pieces2 || col_pieces1 != col_pieces2) {
    fprintf(stderr, "%s: incompatible explosion points, terminating\n", name);
    exit(1);
  }
  /* Now we have all relevant names */
  /* We could now check for addition compatibility */
  /* Or we could assume the user has got it right, */
  /* and only fail when a spawned add fails */
  /* Now create the output names */
  output_map(name, argv[3], col_pieces1, row_pieces1, &names3);
  /* Now loop doing the adds */
  for (i = 0; i < row_pieces1; i++) {
    for (j = 0; j < col_pieces1; j++) {
      add(pathname(argv[1], names1[i * col_pieces1 + j]),
	  pathname(argv[2], names2[i * col_pieces1 + j]),
	  pathname(argv[3], names3[i * col_pieces1 + j]), name);
    }
  }
  return 0;
}
