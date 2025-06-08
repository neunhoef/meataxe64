/*
 * $Id: mtx.c,v 1.11 2005/06/22 21:52:53 jon Exp $
 *
 * Extended row operations for monster meataxe
 *
 */

#include "mtx.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <assert.h>
#include "endian.h"
#include "exrows.h"
#include "map.h"
#include "memory.h"
#include "utils.h"
#include "write.h"

static const char *name = "monst";
static word *row;
static int cv_initialised = 0;
static int initialised = 0;
static FILE **outputs;
static const char **names;

static void quit(const char *a)
{
  fprintf(stderr, "error %s in program monster mtx\n", a);
  exit(15);
}

static void convert_row(u32 total_cols, const unsigned char *bits)
{
  u32 nox = (total_cols + bits_in_word - 1) / bits_in_word;
  u32 i, j, k;
  word x;
  if (0 == cv_initialised) {
    u32 total_fit;
    memory_init(name, 0);
    total_fit = memory_rows(nox, 1000);
    if (0 == total_fit) {
      quit("cannot allocate space for endian conversion");
    }
    row = memory_pointer(0);
    cv_initialised = 1;
  }
  i = 0;
  j = 0;
  k = 0;
  x = 0;
  /* Now convert the incoming row a char at a time in, a word at a time out */
  while (i < (total_cols + (CHAR_BIT) - 1) / (CHAR_BIT)) {
    x |= ((word)(convert_char(bits[i]))) << ((CHAR_BIT) * k);
    k++;
    if (sizeof(word) == k) {
      row[j] = x;
      j++;
      k = 0;
      x = 0;
    }
    i++;
  }
  if (0 != k) {
    row[j] = x;
  }
}

void put_row(u32 row_num, u32 total_cols, u32 split_size, unsigned char *bits)
{
  u32 cols, rows;
  cols = (total_cols + split_size - 1) / split_size;
  rows = cols;
  if (!initialised) {
    initialised = 1;
    output_map(name, ".", cols, rows, &names);
    outputs = my_malloc(cols * sizeof(FILE *));
  }
  convert_row(total_cols, bits);
  ex_row_put(row_num, total_cols, total_cols, ".", names, split_size, row, outputs);
}
