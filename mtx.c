/*
 * $Id: mtx.c,v 1.3 2001/09/18 23:15:46 jon Exp $
 *
 * Extended row operations for monster meataxe
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "endian.h"
#include "header.h"
#include "write.h"
#include "mtx.h"

static int initialised = 0;
static FILE **outputs;
static unsigned long *spaces;

static void quit(const char *a)
{
  fprintf(stderr, "error %s in program monster mtx\n", a);
  exit(15);
}

static void *mymalloc(unsigned long siz)
{
  void *a = (void *)malloc(siz);
  if (NULL == a) {
    fprintf(stderr, "mymalloc fails to obtain %ld bytes\n", siz);
    quit("mymalloc failed");
  }
  return a;
}

static void SSET(unsigned long *s_ptr, unsigned long dim)
{
  *s_ptr = ( (dim+31)/32) * 4;
}

void put_row(unsigned int row_num, unsigned int total_rows, unsigned int split_size, unsigned char *bits)
{
  unsigned int cols = (total_rows + split_size - 1) / split_size;
  unsigned int rows = cols;
  unsigned int i, j;
  if (!initialised) {
    FILE *output = fopen("map", "w");
    initialised = 1;
    if (NULL == output) {
      quit("Cannot open map file");
    }
    spaces = mymalloc(cols * sizeof(unsigned long));
    for (j = 0; j < cols; j++) {
      SSET(spaces + j, (j < cols-1) ? split_size : total_rows - j * split_size);
    }
    fprintf(output, "%6u%6u\n", rows, cols);
    for (i = 0; i < rows; i++) {
      for (j = 0; j < cols; j++) {
	fprintf(output, "%u_%u ", i, j);
      }
      fprintf(output, "\n");
    }
    fclose(output);
  }
  if (row_num % split_size == 0) {
    /* Open relevant files */
    unsigned int row = row_num / split_size;
    unsigned int nor = (row_num + split_size <= total_rows) ? split_size : total_rows - row * split_size;
    outputs = mymalloc(cols * sizeof(FILE *));
    for(i = 0; i < cols; i++) {
      unsigned int noc = ((i+1) * split_size <= total_rows) ? split_size : total_rows - i * split_size;
      char foo[20];
      const header *h = header_create(2, 1, 1, noc, nor);
      sprintf(foo, "%u_%u", row, i);
      outputs[i] = fopen(foo, "wb");
      (void)write_binary_header(outputs[i], h, "monster mtx");
    }
  }
  /* Now output the row */
  for (i = 0; i < cols; i++) {
    unsigned char *v = bits + (i * split_size) / 8;
    (void)endian_write_row(outputs[i], (const unsigned int *)v, spaces[i]);
  }
  /* Now close files if last row for this set */
  if (row_num + 1 == total_rows || (row_num + 1) % split_size == 0) {
    unsigned int i;
    for (i = 0; i < cols; i++) {
      fclose(outputs[i]);
    }
  }
}
