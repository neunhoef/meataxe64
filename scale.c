/*
 * $Id: scale.c,v 1.6 2005/07/24 11:31:35 jon Exp $
 *
 * Function to scale a matrix
 *
 */

#include "scale.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <limits.h>
#include <errno.h>
#include "elements.h"
#include "endian.h"
#include "header.h"
#include "memory.h"
#include "read.h"
#include "rows.h"
#include "utils.h"
#include "write.h"

static int cleanup(FILE *inp, FILE *outp)
{
  if (NULL != inp) {
    fclose(inp);
  }
  if (NULL != outp) {
    fclose(outp);
  }
  return 0;
}

int scale(const char *m1, const char *m2, u32 elt, const char *name)
{
  FILE *inp = NULL;
  FILE *outp = NULL;
  u32 prime, nor, len;
  u32 i;
  const header *h;
  word *row;
  row_ops row_operations;
  row_scaler_in_place scaler;

  if (0 == open_and_read_binary_header(&inp, &h, m1, name)) {
    return 0;
  }
  prime = header_get_prime(h);
  if (1 == prime) {
    fprintf(stderr, "%s: cannot handle maps, terminating\n", name);
    fclose(inp);
    header_free(h);
  }
  nor = header_get_nor(h);
  len = header_get_len(h);
  elt = elt % prime; /* Bring into range */
  if (0 == rows_init(prime, &row_operations)) {
    fprintf(stderr, "%s: cannot initialise row operations for %s, terminating\n", name, m1);
    header_free(h);
    return cleanup(inp, NULL);
  }
  scaler = row_operations.scaler_in_place;
  if (0 == open_and_write_binary_header(&outp, h, m2, name)) {
    header_free(h);
    return cleanup(inp, NULL);
  }
  header_free(h);
  if (memory_rows(len, 1000) < 1) {
    fprintf(stderr, "%s cannot allocate rows for %s, %s, terminating\n", name, m1, m2);
    return cleanup(inp, outp);
  }
  row = memory_pointer_offset(0, 0, len);
  for (i = 0; i < nor; i++) {
    errno = 0;
    if (0 == endian_read_row(inp, row, len)) {
      if ( 0 != errno) {
        perror(name);
      }
      fprintf(stderr, "%s cannot read row %u from %s, terminating\n", name, i, m1);
      return cleanup(inp, outp);
    }
    if (1 != elt) {
      if (0 != elt) {
        (*scaler)(row, len, elt);
      } else {
        row_init(row, len);
      }
    } /* No action if 1 == elt */
    errno = 0;
    if (0 == endian_write_row(outp, row, len)) {
      if ( 0 != errno) {
        perror(name);
      }
      fprintf(stderr, "%s cannot write row %u to %s, terminating\n", name, i, m2);
      return cleanup(inp, outp);
    }
  }
  (void)cleanup(inp, outp);
  return 1;
}
