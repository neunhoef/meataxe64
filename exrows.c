/*
 * $Id: exrows.c,v 1.3 2001/10/09 19:36:26 jon Exp $
 *
 * Extended row manipulation for meataxe
 *
 */

#include <stdio.h>
#include <limits.h>
#include <assert.h>
#include "endian.h"
#include "files.h"
#include "header.h"
#include "utils.h"
#include "write.h"
#include "exrows.h"

int ex_row_put(unsigned int row_num, unsigned int total_cols, unsigned int total_rows,
               const char *dir, const char *names[],
               unsigned int split_size, const unsigned int *row, FILE *outputs[])
{
  unsigned int i;
  unsigned int cols;
  assert(NULL != dir);
  assert(NULL != names);
  assert(NULL != row);
  assert(NULL != outputs);
  split_size = ((split_size + bits_in_unsigned_int - 1)/ bits_in_unsigned_int) * bits_in_unsigned_int;
  cols = (total_cols + split_size - 1) / split_size;
  if (row_num % split_size == 0) {
    /* Open relevant files */
    unsigned int row = row_num / split_size;
    unsigned int nor = (row_num + split_size <= total_rows) ? split_size : total_rows - row * split_size;
    for(i = 0; i < cols; i++) {
      unsigned int noc = ((i+1) * split_size <= total_cols) ? split_size : total_cols - i * split_size;
      const header *h = header_create(2, 1, 1, noc, nor);
      assert(NULL != h);
      outputs[i] = fopen(pathname(dir, names[row * cols + i]), "wb");
      (void)write_binary_header(outputs[i], h, "monster mtx");
    }
  }
  /* Now output the row */
  for (i = 0; i < cols; i++) {
    const unsigned int *v = row + (i * split_size) / bits_in_unsigned_int;
    unsigned int size = (i + 1 < cols) ? split_size : total_cols - i * split_size;
    size = (size + bits_in_unsigned_int - 1) / bits_in_unsigned_int;
    (void)endian_write_row(outputs[i], v, size);
  }
  /* Now close files if last row for this set */
  if (row_num + 1 == total_rows || (row_num + 1) % split_size == 0) {
    unsigned int i;
    for (i = 0; i < cols; i++) {
      fclose(outputs[i]);
    }
  }
  return 0;
}
