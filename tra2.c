/*
 * $Id: tra2.c,v 1.2 2012/03/24 13:31:32 jon Exp $
 *
 * Function to transpose a matrix
 *
 */

#include "tra2.h"
#include <stdio.h>
#include <assert.h>
#include <errno.h>
#include "elements.h"
#include "endian.h"
#include "header.h"
#include "maps.h"
#include "matrix.h"
#include "memory.h"
#include "read.h"
#include "rows.h"
#include "utils.h"
#include "write.h"

static u32 masks1[5] =
  {
    0x0000ffff, /* For 16 x 16 */
    0x00ff00ff, /* for 8 x 8 */
    0x0f0f0f0f, /* for 4 x 4 */
    0x33333333, /* for 2 x 2 */
    0x55555555  /* for 1 x 1 */
  };


static u32 masks2[5] =
  {
    0xffff0000, /* For 16 x 16 */
    0xff00ff00, /* for 8 x 8 */
    0xf0f0f0f0, /* for 4 x 4 */
    0xcccccccc, /* for 2 x 2 */
    0xaaaaaaaa  /* for 1 x 1 */
  };

int tra2(const char *m1, const char *m2, const char *name)
{
  FILE *input;
  FILE *output;
  u32 nob, noc, nor, prime, len1, len2, len_in, max, total, t1, t2;
  u32 i, j, k;
  /* These next two define the top left hand corner of a brick */
  u32 in_col; /* The input column we started working at this time */
  u32 in_row; /* The input row we started working at this time */
  const header *h1, *h2;
  word **rows_in, **rows_out;
  s64 pos;

  assert(NULL != m1);
  assert(NULL != m2);
  assert(NULL != name);
  /* Try to read the input */
  if (0 == open_and_read_binary_header(&input, &h1, m1, name)) {
    return 0;
  }
  prime = header_get_prime(h1);
  nor = header_get_nor(h1);
  noc = header_get_noc(h1);
  if (1 == prime) {
    /* Handle is a map */
    int ret = map_iv(input, h1, m1, m2, name);
    header_free(h1);
    fclose(input);
    return ret;
  }
  nob = header_get_nob(h1);
  len1 = header_get_len(h1);
  /* Create header for transpose */
  h2 = header_create(prime, nob, header_get_nod(h1), nor, noc);
  header_free(h1);
  len2 = header_get_len(h2);
  /* Maximum row size of input and output */
  max = (len1 > len2) ? len1 : len2;
  /* Write the output header */
  if (0 == open_and_write_binary_header(&output, h2, m2, name)) {
    fclose(input);
    header_free(h2);
    return 0;
  }
  header_free(h2);
  /*
   * We want to take a brick of A and transpose that into the right
   * part of B. Suppose we have n1 columns of A and n2 rows
   * where 32 | (n1,n2) (smaller left over bits can be handled separately)
   * We need n1 rows of length nor for the transpose output,
   * and n2 rows of length n1 for the input
   * If we can fit 32 full output rows we should do so
   */
  /*
   * Start by seeing how many rows of length nor (len2)
   * we can fit in half the memory. These will be the output rows
   */
  total = memory_rows(len2, 500);
  if (total < 32) {
    /* Eventually we must handle this, but for the moment give up */
    /*
     * Bit crap, since we expect say up to 4 million output rows
     * so 16 Mb is required
     */
    fprintf(stderr, "%s cannot allocate 32 rows of length %d for %s, %s, terminating\n", name, len2, m1, m2);
    fclose(input);
    fclose(output);
    return 0;
  }
  /* Now trim total to a multiple of 32 */
  total -= (total % 32);
  /* Don't do more rwos out than there are */
  total = (total > noc) ? noc : total;
  /* We need to convert total into words for columns */
  len_in = compute_len(nob, total);
  /* Now see what we can fit the other way round (input rows) */
  t1 = memory_rows(len_in, 500); /* Should do better than this / 32 */
  if (t1 < 32) {
    /* Eventually we must handle this, but for the moment give up */
    /*
     * Bit crap, since we expect say up to 4 million output rows
     * so 16 Mb is required
     */
    fprintf(stderr, "%s cannot allocate 32 rows of length %d for %s, %s, terminating\n", name, len_in, m1, m2);
    fclose(input);
    fclose(output);
    return 0;
  }
  t1 -= (t1 % 32);
  printf("Working with %d output rows of length %d and %d input rows of length %d\n", total, len2, t1, len_in);
  /* Don't try to do more rows than there are */
  if (t1 > nor) {
    /* TBD we need to blank the extra rows so they can participate */
    t2 = nor + 31;
    t2 -= t2 % 32;
    t1 = nor;
  } else {
    t2 = t1;
  }
  /* Allocate the rows for input */
  rows_in = matrix_malloc(t2);
  /* Allocate the rows for output */
  rows_out = matrix_malloc(total);
  /* The rows for the input */
  for (i = 0; i < t2; i++) {
    rows_in[i] = memory_pointer_offset(0, i, len_in);
  }
  /* The rows for the output */
  for (i = 0; i < total; i++) {
    rows_out[i] = memory_pointer_offset(500, i, len2);
  }
  /*
   * We want to do input rows in sets of 32
   * as we're going to transpose 32 rows by 32 bits
   * in 5 rounds of shifts, masks and ors
   */
  /* Loop over vertical divides of input matrix */
  /* Start at the left hand edge of m1 */
  for (in_col = 0; in_col < noc; in_col += total) {
    u32 last_col = in_col + total;
    /* How much to skip at start */
    u32 row_start_skip = compute_len(nob, in_col);
    /* How much to skip at end */
    u32 row_end_skip;
    u32 row_length;
    last_col = (last_col > noc) ? noc : last_col;
    row_end_skip = compute_len(nob, noc - last_col);
    row_length = (0 == row_end_skip) ? compute_len(nob, last_col - in_col) : len_in;
    last_col = (last_col > noc) ? noc : last_col;
    printf("Handling input from column %d to %d\n", in_col, last_col);
    printf("row_length = %d, row_start_skip = %d, row_end_skip = %d\n", row_length, row_start_skip, row_end_skip);
    /* There is no horizontal divide, from our choice of t1 */
    /* Read the bits of row */
    pos = ftello64(input);
    printf("pos = %lld\n", pos);
    /*
     * TBD, we need to handle the case where nor isn't a multiple of 32
     * by creating some blank rows
     */
    for (k = 0; k < nor; k++) {
      printf("Reading row %d from %lld\n", k, ftello64(input));
      /* Skip start of row */
      if (0 != row_start_skip) {
        endian_skip_row(input, row_start_skip);
      }
      printf("Reading data for row %d from %lld\n", k, ftello64(input));
      if (0 == endian_read_row(input, rows_in[k], row_length)) {
        if ( 0 != errno) {
          perror(name);
        }
        fprintf(stderr, "%s cannot read row %u from %s, terminating\n", name, k, m1);
        fclose(input);
        fclose(output);
        matrix_free(rows_in);
        matrix_free(rows_out);
        return 0;
      }
      {
        u32 nz;
        u32 e = first_non_zero(rows_in[k], nob, row_length, &nz);
        printf("First non zero %d for row %d found at %d\n", e, k, nz);
      }
      /* Move input into output */
      /* Five rounds of block transpose with smaller and smaller blocks */
      /* Now need to skip rest of row */
      if (0 != row_end_skip) {
        endian_skip_row(input, row_end_skip);
      }
    }
    /* Now work within the large brick, 32 rows at a time */
    for (in_row = 0; in_row < nor; in_row += 32) {
      u32 last_row = in_row + 32;
      u32 out_word_pos = compute_len(nob, in_row);
      printf("out_word_pos = %d\n", out_word_pos);
      /* And work across the brick, 32 columns at a time */
      for (k = 0; k < len_in; k++) {
        int phase = 0;
        unsigned int shift = 16;
        u32 mask1 = masks1[phase];
        u32 mask2 = masks2[phase];
        word elts[32]; /* These are the elements we'll work with */

        /* Our current input 32 x 32 block lives at (in_row, in_col+32k) */
        /* The output goes at (in_col+32k, in_row) */
        /* printf("Handling brick (%d, %d), moving to (%d, %d)\n", in_row, in_col+32*k, in_col+32*k, in_row); */
        /* Now we need to do the transpose algorithm */
        /* Set up the curent elements */
        for (i = 0; i < 32; i++) {
          u32 nz;
          u32 e = first_non_zero(rows_in[in_row + i], nob, row_length, &nz);
          printf("First non zero %d for row %d found at %d\n", e, in_row + i, nz);
          if (e != 0) {
            printf("Element found was 0x%x\n", rows_in[in_row + i][0]);
          }
        }
        for (i = 0; i < 32; i++) {
          elts[i] = rows_in[in_row + i][k];
          printf("row %d, column %d elt 0x%x\n", in_row + i, k * 32, elts[i]);
        }
        /* We'll transpose in situ, then copy back at the end */
        /* Phase 1: transpose a 32 x 32 matrix in 16 x 16 blocks */
        for (i = 0; i < shift; i++) {
          word new1 = (elts[i] & mask1) | ((elts[i + shift] & mask1) << shift);
          word new2 = (elts[i + shift] & mask2) | ((elts[i] & mask2) >> shift);
          printf("Setting elts[%d] to 0x%x from 0x%x\n", i, elts[i], new1);
          elts[i] = new1;
          printf("Setting elts[%d] to 0x%x from 0x%x\n", i + shift, elts[i + shift], new2);
          elts[i + shift] = new2;
        }
        phase++;
        shift >>= 1;
        mask1 = masks1[phase];
        mask2 = masks2[phase];
        /* Phase 2: transpose a 32 x 32 matrix in 4 8 x 8 blocks */
        for (j = 0; j < 2; j++) {
          for (i = 0; i < shift; i++) {
            u32 l = 2 * shift * j + i; /* The row we work with */
            word new1 = (elts[l] & mask1) | ((elts[l + shift] & mask1) << shift);
            word new2 = (elts[l + shift] & mask2) | ((elts[l] & mask2) >> shift);
            printf("Setting elts[%d] to 0x%x from 0x%x\n", l, elts[l], new1);
            elts[l] = new1;
            printf("Setting elts[%d] to 0x%x from 0x%x\n", l + shift, elts[l + shift], new2);
            elts[l + shift] = new2;
          }
        }
        /* Phase 3: transpose a 32 x 32 matrix in 16 4 x 4 blocks */
        /* Phase 4: transpose a 32 x 32 matrix in 64 2 x 2 blocks */
        /* Phase 5: transpose a 32 x 32 matrix in 256 1 x 1 blocks */
        /* Now copy back from elts to (in_col+32k, in_row) */
        for (i = 0; i < 32; i++) {
          printf("Setting out row %d column %d to 0x%x\n", in_col + i + 32 * k, out_word_pos * 32, elts[i]);
          rows_out[i + 32 * k][out_word_pos] = elts[i];
        }
      }
      NOT_USED(last_row);
    }
    /* All read, return to start */
    if (0 != fseeko64(input, pos, SEEK_SET)) {
      fprintf(stderr, "%s: unable to rewind %s, terminating\n", name, m1);
      fclose(input);
      fclose(output);
      matrix_free(rows_in);
      matrix_free(rows_out);
      return 0;
    }
    /* Now write out the result rows */
    for (j = 0; j < last_col - in_col; j++) {
      errno = 0;
      if (0 == endian_write_row(output, rows_out[j], len2)) {
        if ( 0 != errno) {
          perror(name);
        }
        fprintf(stderr, "%s cannot write row %u to %s, terminating\n", name, j, m2);
        fclose(input);
        fclose(output);
        matrix_free(rows_in);
        matrix_free(rows_out);
        return 0;
      }
    }
  }
  fclose(input);
  fclose(output);
  matrix_free(rows_in);
  matrix_free(rows_out);
  return 1;
}
