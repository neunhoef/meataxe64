/*
 * $Id: etr.c,v 1.2 2002/01/06 16:35:48 jon Exp $
 *
 * Exploded transpose
 *
 * Two arguments
 * 1) input dir
 * 2) output dir
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "endian.h"
#include "files.h"
#include "map.h"
#include "memory.h"
#include "tra.h"
#include "utils.h"

static const char *name = "etr";

static void etr_usage(void)
{
  fprintf(stderr, "%s: usage: %s <dir1> <dir2> <dir3>\n", name, name);
}

int main(int argc,  const char *const argv[])
{
  unsigned int col_pieces, row_pieces;
  unsigned int i, j;
  const char **names1, **names2;
  memory_init(name, 0);
  endian_init();
  /******  First check the number of input arguments  */
  if (argc != 3) {
    etr_usage();
    exit(1);
  }
  /* Now get a look at the map file */
  input_map(name, argv[1], &col_pieces, &row_pieces, &names1);
  /* Now we have all relevant names */
  /* We could now check for addition compatibility */
  /* Or we could assume the user has got it right, */
  /* and only fail when a spawned add fails */
  /* Now create the output names */
  output_map(name, argv[2], row_pieces, col_pieces, &names2);
  /* Now loop doing the transposes */
  for (i = 0; i < row_pieces; i++) {
    for (j = 0; j < col_pieces; j++) {
      tra(pathname(argv[1], names1[i * col_pieces + j]),
          pathname(argv[2], names2[j * row_pieces + i]), name);
    }
  }
  return 0;
}
