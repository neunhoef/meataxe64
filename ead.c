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
#include "memory.h"
#include "utils.h"

static const char *name = "ead";

static void ead_usage(void)
{
  fprintf(stderr, "%s: usage: %s <dir1> <dir2> <dir3>\n", name, name);
}

static char *pathname(const char *dirname, const char *filename)
{
  char *result = my_malloc(strlen(dirname) + strlen(filename) + 2);
  strcpy(result, dirname);
  strcat(result, "/");
  strcat(result, filename);
  return result;
}

static long hadcr = 0;

/******  subroutine to get an integer like FORTRAN does  */
static unsigned long getin(FILE *f, unsigned long a)
{
  int c;
  unsigned long i,j=0;
 
  if(hadcr == 1) return j;
  for(i=0;i<a;i++) {
    c = fgetc(f);
    if(c == '\n') {
      hadcr = 1;
      return j;
    }
    if(c < '0') c = '0';
    if(c > '9') c = '0';
    j = 10*j + (c-'0');
  }
  return j;
}
 
static void nextline(FILE *f)
{
  if(hadcr == 1) {
    hadcr=0;
    return;
  }
  while (fgetc(f) != '\n');
}
 
static int isspace(int i)
{
    return (i == ' ') || (i == 9) || (i == 10) || (i == 13);
}

static char *get_str(FILE *f, char **name, unsigned int depth)
{
  int c = fgetc(f);
  if (isspace(c)) {
    *name = my_malloc(depth + 1);
    (*name)[depth] = '\0';
    return *name;
  } else {
    get_str(f, name, depth + 1);
    (*name)[depth] = c;
    return *name;
  }
}

int main(int argc,  char **argv)
{
  FILE *input1, *input2;
  FILE *output;
  unsigned long col_pieces1, row_pieces1;
  unsigned long col_pieces2, row_pieces2;
  unsigned long i, j;
  char **names;
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
  fprintf(output, "%6lu%6lu\n", row_pieces1, col_pieces1);
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
