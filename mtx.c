/*
 * $Id: mtx.c,v 1.5 2001/09/30 17:51:20 jon Exp $
 *
 * Extended row operations for monster meataxe
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <assert.h>
#include "endian.h"
#include "header.h"
#include "memory.h"
#include "utils.h"
#include "write.h"
#include "mtx.h"

static const char *name = "monst";
static unsigned char *row;
static unsigned char table[256];
static int cv_initialised = 0;
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
  *s_ptr = ( (dim+31)/32);
}

static void convert_byte(unsigned char *out, const unsigned char *in)
{
  *out = table[*in];
}

static void convert_row(unsigned int total_cols, const unsigned char *bits)
{
  unsigned int nox = (total_cols + bits_in_unsigned_int - 1) / bits_in_unsigned_int;
  unsigned int i;
  if (0 == cv_initialised) {
    unsigned int total_fit;
    memory_init(name, 0);
    total_fit = memory_rows(nox, 1000);
    if (0 == total_fit) {
      quit("cannot allocate space for endian conversion");
    }
    row = memory_pointer(0);
    /* Now build the conversion table */
    for (i = 0; i < 256; i++) {
      unsigned int j = i, k = 0, l;
      for (l = 0; l < 8; l++) {
        k = (k << 1) | (j & 1);
        j >>= 1;
      }
      table[i] = k;
    }
    cv_initialised = 1;
  }
  /* Now convert the incoming row a word at a time */
  for (i = 0; i < nox * sizeof(unsigned int); i += 1) {
    convert_byte(row + i, bits + i);
  }
}

void put_row(unsigned int row_num, unsigned int total_cols, unsigned int split_size, unsigned char *bits)
{
  unsigned int cols;
  unsigned int rows;
  unsigned int total_rows = total_cols;
  unsigned int i, j;
  split_size = ((split_size + bits_in_unsigned_int - 1)/ bits_in_unsigned_int) * bits_in_unsigned_int;
  cols = (total_cols + split_size - 1) / split_size;
  rows = cols;
  convert_row(total_cols, bits);
  if (!initialised) {
    FILE *output = fopen("map", "w");
    initialised = 1;
    if (NULL == output) {
      quit("Cannot open map file");
    }
    spaces = mymalloc(cols * sizeof(unsigned long));
    for (j = 0; j < cols; j++) {
      SSET(spaces + j, (j < cols-1) ? split_size : total_cols - j * split_size);
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
      unsigned int noc = ((i+1) * split_size <= total_cols) ? split_size : total_cols - i * split_size;
      char foo[20];
      const header *h = header_create(2, 1, 1, noc, nor);
      sprintf(foo, "%u_%u", row, i);
      outputs[i] = fopen(foo, "wb");
      (void)write_binary_header(outputs[i], h, "monster mtx");
    }
  }
  /* Now output the row */
  for (i = 0; i < cols; i++) {
    unsigned char *v = row + (i * split_size) / 8;
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
