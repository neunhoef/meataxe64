/* ead.c */
/*
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
#include "files.h"
#include "memory.h"
#include "utils.h"

static const char *name = "ead";

static void ead_usage(void)
{
  fprintf(stderr, "%s: usage: %s <dir1> <dir2> <dir3>\n", name, name);
}

int main(int argc,  char **argv)
{
  FILE *input1, *input2;
  FILE *output;
  unsigned int col_pieces1, row_pieces1;
  unsigned int col_pieces2, row_pieces2;
  unsigned int i, j;
  const char **names;
  char *temp;
  const char *m1, *m2, *m3;
  memory_init(name, 0);
  /******  First check the number of input arguments  */
  if (argc != 4) {
    ead_usage();
    exit(1);
  }
  /* Now get a look at the map file */
  m1 = pathname(argv[1], "map");
  input1 = fopen(m1, "rb");
  if (input1 == NULL) {
    fprintf(stderr, "%s: cannot open first input map %s", name, m1);
    exit(1);
  }
  m2 = pathname(argv[2], "map");
  input2 = fopen(m2, "rb");
  if (input2 == NULL) {
    fprintf(stderr, "%s: cannot open second input map %s", name, m2);
    exit(1);
  }
  row_pieces1 = getin(input1, 6);
  col_pieces1 = getin(input1, 6);
  names = my_malloc(row_pieces1 * col_pieces1 * sizeof(char *));
  nextline(input1);
  row_pieces2 = getin(input2, 6);
  col_pieces2 = getin(input2, 6);
  if (row_pieces1 != row_pieces2 || col_pieces1 != col_pieces2) {
    fprintf(stderr, "%s: incompatible explosion points, terminating\n", name);
    exit(1);
  }
  fclose(input2);
  for (i = 0; i < row_pieces1; i++) {
    for (j = 0; j < col_pieces1; j++) {
      names[i * col_pieces1 + j] = get_str(input1, &temp, 0);
    }
    nextline(input1);
  }
  fclose(input1);
  /* Now we have all relevant names */
  /* We could now check for addition compatibility */
  /* Or we could assume the user has got it right, */
  /* and only fail when a spawned add fails */

  /* Now create the result map file */
  /* Same as the first file */
  m3 = pathname(argv[3], "map");
  output = fopen(m3, "wb");
  if (output == NULL) {
    fprintf(stderr, "%s: cannot open output map %s", name, m3);
    exit(1);
  }
  fprintf(output, "%6u%6u\n", row_pieces1, col_pieces1);
  for (i = 0; i < row_pieces1; i++) {
    for (j = 0; j < col_pieces1; j++) {
      fprintf(output, "%s ", names[i * col_pieces1 + j]);
    }
    fprintf(output, "\n");
  }
  fclose(output);
  /* Now loop doing the adds */
  for (i = 0; i < row_pieces1; i++) {
    for (j = 0; j < col_pieces1; j++) {
      add(pathname(argv[1], names[i * col_pieces1 + j]),
	  pathname(argv[2], names[i * col_pieces1 + j]),
	  pathname(argv[3], names[i * col_pieces1 + j]), name);
    }
  }
  return 0;
}
