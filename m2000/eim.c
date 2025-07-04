/*
 * $Id: eim.c,v 1.15 2006/12/04 22:33:24 jon Exp $
 *
 * implode a matrix (ie glue exploded matrices together)
 *
 * Based on version 6.0.1 20/10/98
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#include "elements.h"
#include "endian.h"
#include "exrows.h"
#include "files.h"
#include "header.h"
#include "memory.h"
#include "map.h"
#include "parse.h"
#include "primes.h"
#include "read.h"
#include "utils.h"
#include "write.h"

static const char *name = "eim";

static void eim_usage(void)
{
  fprintf(stderr, "%s: usage: %s %s <mat> <dir>\n", name, name, parse_usage());
}

static void fail(FILE **inputs, FILE *output, u32 cols)
{
  u32 i;
  assert(NULL != output);
  assert(NULL != inputs);
  for (i = 0; i < cols; i++) {
    assert(NULL != inputs[i]);
    fclose(inputs[i]);
  }
  fclose(output);
  exit(1);
}

int main(int argc,  const char *const argv[])
{
  FILE *input = NULL, *output = NULL;
  FILE **inputs;
  const char *matrix;
  const header *h = NULL, *outh;
  const header **headers;
  word *row1, *row2;
  u32 nob = 0, nod = 0, nor, len, prime = 0;
  u32 col_pieces, row_pieces;
  u32 rows, cols;
  u32 i, j;
  const char **names;
  argv = parse_line(argc, argv, &argc);
  if (3 != argc) {
    eim_usage();
    exit(1);
  }
  endian_init();
  memory_init(name, memory);
  /* Now get a look at the map file */
  input_map(name, argv[2], &col_pieces, &row_pieces, &names);
  /* Now determine full size */
  rows = 0;
  cols = 0;
  for (i = 0; i < col_pieces; i++) {
    matrix = pathname(argv[2], names[i]);
    if (0 == open_and_read_binary_header(&input, &h, matrix, name)) {
      exit(1);
    }
    assert(NULL != h);
    fclose(input);
    cols += header_get_noc(h);
    header_free(h);
  }
  for (i = 0; i < row_pieces; i++) {
    matrix = pathname(argv[2], names[i * col_pieces]);
    if (0 == open_and_read_binary_header(&input, &h, matrix, name)) {
      exit(1);
    }
    assert(NULL != h);
    fclose(input);
    rows += header_get_nor(h);
    if (0 == i) {
      prime = header_get_prime(h);
      nob = header_get_nob(h);
      nod = header_get_nod(h);
    }
    header_free(h);
  }
  if (1 == prime) {
    fprintf(stderr, "%s: cannot handle maps, terminating\n", name);
    exit(1);
  }
  assert(0 != nob);
  assert(0 != nod);
  assert(is_a_prime_power(prime));
  outh = header_create(prime, nob, nod, cols, rows);
  assert(NULL != outh);
  len = header_get_len(outh);
  if (0 == open_and_write_binary_header(&output, outh, argv[1], name)) {
    exit(1);
  }
  /* Now copy all the input matrices into the output one */
  if (memory_rows(len, 1000) < 2) {
    fprintf(stderr, "%s cannot allocate rows for %s, terminating\n", name, argv[1]);
    fclose(output);
    exit(1);
  }
  row1 = memory_pointer_offset(0, 0, len);
  row2 = memory_pointer_offset(0, 1, len);
  inputs = my_malloc(row_pieces * sizeof(*inputs));
  headers = my_malloc(row_pieces * sizeof(*headers));
  for (i = 0; i < row_pieces; i++) {
    u32 k;
    for (j = 0; j < col_pieces; j++) {
      const char *piece_name = pathname(argv[2], names[i * row_pieces + j]);
      if (0 == open_and_read_binary_header(inputs + j, headers + j, piece_name, name)) {
        fail(inputs, output, j + 1);
      }
      assert(NULL != headers[j]);
      if (1 == header_get_prime(headers[j])) {
        fprintf(stderr, "%s: cannot handle maps, terminating\n", name);
        exit(1);
      }
    }
    /* Now check for consistent headers */
    nor = header_get_nor(headers[0]);
    for (j = 1; j < col_pieces; j++) {
      if (header_get_nor(headers[j]) != nor || header_get_prime(headers[j]) != prime) {
        fprintf(stderr, "%s header mismatch %u != %u or %u != %u from %s and %s, terminating\n",
                name, nor, header_get_nor(headers[j]), prime, header_get_prime(headers[j]),
                pathname(argv[2], names[i * col_pieces]), pathname(argv[2], names[i * col_pieces + j]));
        fail(inputs, output, col_pieces);
      }
    }
    for (k = 0; k < nor; k++) {
      if (0 == ex_row_get(col_pieces, inputs, headers, row1, row2, name, names, i, nob)) {
        fail(inputs, output, col_pieces);
      }
      errno = 0;
      if (0 == endian_write_row(output, row1, header_get_len(outh))) {
        if ( 0 != errno) {
          perror(name);
        }
        fprintf(stderr, "%s cannot write row to %s, terminating\n",
                name, argv[1]);
        fail(inputs, output, col_pieces);
      }
    }
    /* Now close the current set and continue */
    for (j = 0; j < col_pieces; j++) {
      fclose(inputs[j]);
      header_free(headers[j]);
    }
  }
  fclose(output);
  memory_dispose();
  return 0;
}
