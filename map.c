/*
 * $Id: map.c,v 1.2 2001/10/07 18:02:56 jon Exp $
 *
 * Handle maps for exploded matrices
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "files.h"
#include "utils.h"
#include "map.h"

void input_map(const char *name, const char *dir, unsigned int *cols,
               unsigned int *rows, const char ***names)
{
  const char *m;
  FILE *input;
  unsigned int row, col, i, j;
  assert(NULL != name);
  assert(NULL != dir);
  assert(NULL != cols);
  assert(NULL != rows);
  assert(NULL != names);
  m = pathname(dir, "map");
  input = fopen(m, "rb");
  if (NULL == input) {
    fprintf(stderr, "%s: cannot open input map %s\n", name, m);
    exit(1);
  }
  row = getin(input, 6);
  col = getin(input, 6);
  *names = my_malloc(row * col * sizeof(char *));
  for (i = 0; i < row; i++) {
    for (j = 0; j < col; j++) {
      (*names)[i * col + j] = get_str(input);
    }
  }
  fclose(input);
  *cols = col;
  *rows = row;
}

void output_map(const char *name, const char *dir, unsigned int cols,
                unsigned int rows, const char ***names)
{
  const char *m;
  FILE *output;
  unsigned int i, j, name_size;
  assert(NULL != name);
  assert(NULL != dir);
  assert(NULL != names);
  *names = my_malloc(rows * cols * sizeof(char *));
  name_size = digits_of(rows) + digits_of(cols) + 2; /* nnn_mmm\0 */
  for (i = 0; i < rows; i++) {
    for (j = 0; j < cols; j++) {
      char *name = my_malloc(name_size); /* Make this more efficient ****/
      sprintf(name, "%u_%u", i, j);
      (*names)[ i * cols + j] = name;
    }
  }
  m = pathname(dir, "map");
  output = fopen(m, "wb");
  if (output == NULL) {
    fprintf(stderr, "%s: cannot open output map %s\n", name, m);
    exit(1);
  }
  fprintf(output, "%6u%6u\n", rows, cols);
  for (i = 0; i < rows; i++) {
    for (j = 0; j < cols; j++) {
      fprintf(output, "%s ", (*names)[i * cols + j]);
    }
    fprintf(output, "\n");
  }
  fclose(output);
}
