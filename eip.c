/*
 * $Id: eip.c,v 1.1 2001/10/18 22:59:18 jon Exp $
 *
 * Read a permutation into an exploded matrix
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <limits.h>
#include "elements.h"
#include "endian.h"
#include "exrows.h"
#include "header.h"
#include "map.h"
#include "memory.h"
#include "read.h"
#include "rows.h"
#include "utils.h"
#include "write.h"

static const char *name = "eip";

static void eip_usage(void)
{
  fprintf(stderr, "%s: usage: %s <in_file> <out_dir> <prime> <split>\n", name, name);
}

int main(int argc, const char * const argv[])
{
  const char *in;
  const char *out;
  FILE *inp;
  FILE **outputs;
  unsigned int prime, nob, nod, nor, len;
  unsigned int row_pieces, split;
  unsigned int elts_per_word;
  const char **names;
  unsigned int i;
  unsigned int t1, t2, t3;
  const header *h;
  unsigned int *row;

  if (5 != argc) {
    eip_usage();
    exit(1);
  }
  in = argv[1];
  out = argv[2];
  prime = strtoul(argv[3], NULL, 0);
  split = strtoul(argv[4], NULL, 0);
  if (0 == is_a_prime_power(prime)) {
    fprintf(stderr, "%s: %d is not a prime power, terminating\n", name, prime);
    exit(1);
  }
  if (0 == split) {
    fprintf(stderr, "%s: %d is not an acceptable split, terminating\n", name, split);
    exit(1);
  }
  nob = bits_of(prime);
  nod = digits_of(prime);
  inp = fopen(in, "r");
  if (NULL == inp) {
    fprintf(stderr, "%s: cannot open %s, terminating\n", name, in);
    exit(1);
  }
  endian_init();
  memory_init(name, 0);
  if (0 == read_text_header_items(inp, &t1, &t2, &nor, &t3, in)) {
    fclose(inp);
    exit(1);
  }
  elts_per_word = bits_in_unsigned_int / nob;
  /* Now compute the number of pieces for both rows and columns */
  /* And produce the description file */
  /* Align split to word boundary */
  split = ((split + elts_per_word - 1) / elts_per_word) * elts_per_word;
  row_pieces = (nor + split - 1) / split;
  outputs = my_malloc(row_pieces * sizeof(FILE *));
  output_map(name, argv[2], row_pieces, row_pieces, &names);
  h = header_create(prime, nob, nod, nor, nor);
  len = header_get_len(h);
  if (memory_rows(len, 1000) < 1) {
    fprintf(stderr, "%s: cannot create output row\n", name);
    fclose(inp);
    return 0;
  }
  row = memory_pointer_offset(0, 0, len);
  assert(NULL != row);
  for (i = 0; i < nor; i++) {
    unsigned int j;
    row_init(row, len);
    j = getin(inp, 7);
    assert(j >= 1);
    put_element_to_row(nob, j - 1, row, 1);
    ex_row_put(i, nor, nor, argv[2], names, split, row, outputs);
  }
  fclose(inp);
  return 0;
}
