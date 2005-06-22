/*
 * $Id: exrows.c,v 1.10 2005/06/22 21:52:53 jon Exp $
 *
 * Extended row manipulation for meataxe
 *
 */

#include <stdio.h>
#include <limits.h>
#include <assert.h>
#include <errno.h>
#include "elements.h"
#include "endian.h"
#include "files.h"
#include "header.h"
#include "utils.h"
#include "write.h"
#include "exrows.h"

int ex_row_put(u32 row_num, u32 total_cols, u32 total_rows,
               const char *dir, const char *names[],
               u32 split_size, const word *row, FILE *outputs[])
{
  u32 i;
  u32 cols;
  assert(NULL != dir);
  assert(NULL != names);
  assert(NULL != row);
  assert(NULL != outputs);
  split_size = ((split_size + bits_in_word - 1)/ bits_in_word) * bits_in_word;
  cols = (total_cols + split_size - 1) / split_size;
  if (row_num % split_size == 0) {
    /* Open relevant files */
    u32 row = row_num / split_size;
    u32 nor = (row_num + split_size <= total_rows) ? split_size : total_rows - row * split_size;
    for(i = 0; i < cols; i++) {
      u32 noc = ((i+1) * split_size <= total_cols) ? split_size : total_cols - i * split_size;
      const header *h = header_create(2, 1, 1, noc, nor);
      assert(NULL != h);
      (void)open_and_write_binary_header(outputs + i, h, pathname(dir, names[row * cols + i]),
                                         "monster mtx");
      header_free(h);
    }
  }
  /* Now output the row */
  for (i = 0; i < cols; i++) {
    const word *v = row + (i * split_size) / bits_in_word;
    u32 size = (i + 1 < cols) ? split_size : total_cols - i * split_size;
    size = (size + bits_in_word - 1) / bits_in_word;
    (void)endian_write_row(outputs[i], v, size);
  }
  /* Now close files if last row for this set */
  if (row_num + 1 == total_rows || (row_num + 1) % split_size == 0) {
    u32 i;
    for (i = 0; i < cols; i++) {
      fclose(outputs[i]);
    }
  }
  return 0;
}

int ex_row_get(u32 col_pieces, FILE **inputs, const header **headers,
               word *row1, word *row2, const char *name, const char **names,
               u32 i, u32 nob)
{
  u32 j, elts_per_word, l = 0; /* Index into output row */
  word mask = get_mask_and_elts(nob, &elts_per_word);
  for (j = 0; j < col_pieces; j++) {
    u32 m = 0, n;
    errno = 0;
    if (0 == endian_read_row(inputs[j], row2, header_get_len(headers[j]))) {
      if ( 0 != errno) {
        perror(name);
      }
      fprintf(stderr, "%s cannot read row from %s, terminating\n",
              name, names[i * col_pieces + j]);
      return 0;
    }
    n = header_get_noc(headers[j]);
    while (m < n) {
      put_element_to_row_with_params(nob, l, mask, elts_per_word, row1,
                                     get_element_from_row_with_params(nob, m, mask, elts_per_word, row2));            
      m++;
      l++;
    }
  }
  return 1;
}
