/*
 * $Id: mtx.c,v 1.6 2001/10/03 00:01:42 jon Exp $
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
#include "exrows.h"
#include "header.h"
#include "map.h"
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
static const char **names;

static void quit(const char *a)
{
  fprintf(stderr, "error %s in program monster mtx\n", a);
  exit(15);
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
  unsigned int cols, rows;
  cols = (total_cols + split_size - 1) / split_size;
  rows = cols;
  if (!initialised) {
    initialised = 1;
    output_map(name, ".", cols, rows, &names);
    outputs = my_malloc(cols * sizeof(FILE *));
  }
  convert_row(total_cols, bits);
  ex_row_put(row_num, total_cols, ".", names, split_size, (unsigned int *)row, outputs);
}
