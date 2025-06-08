/*
 * $Id: eip.c,v 1.14 2011/01/19 22:47:16 jon Exp $
 *
 * Read a permutation into an exploded matrix
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <limits.h>
#include <errno.h>
#include "elements.h"
#include "endian.h"
#include "exrows.h"
#include "header.h"
#include "map.h"
#include "memory.h"
#include "parse.h"
#include "primes.h"
#include "read.h"
#include "rows.h"
#include "utils.h"
#include "write.h"

static const char *name = "eip";

static void eip_usage(void)
{
  fprintf(stderr, "%s: usage: %s %s <in_file> <out_dir> <field order> <split>\n", name, name, parse_usage());
}

int main(int argc, const char * const argv[])
{
  const char *in;
  const char *out;
  FILE *inp;
  FILE **outputs;
  u32 prime, nob, nod, nor, len;
  u32 row_pieces, split;
  u32 elts_per_word;
  const char **names;
  u32 i;
  u32 t1, t2, t3;
  const header *h;
  word *row;

  argv = parse_line(argc, argv, &argc);
  if (5 != argc) {
    eip_usage();
    exit(1);
  }
  in = argv[1];
  out = argv[2];
  prime = strtoul(argv[3], NULL, 0);
  split = strtoul(argv[4], NULL, 0);
  if (0 == is_a_prime_power(prime)) {
    fprintf(stderr, "%s: %u is not a prime power, terminating\n", name, prime);
    exit(1);
  }
  if (0 == split) {
    fprintf(stderr, "%s: %u is not an acceptable split, terminating\n", name, split);
    exit(1);
  }
  nob = bits_of(prime);
  nod = digits_of(prime);
  errno = 0;
  inp = fopen(in, "r");
  if (NULL == inp) {
    if ( 0 != errno) {
      perror(name);
    }
    fprintf(stderr, "%s: cannot open %s, terminating\n", name, in);
    exit(1);
  }
  endian_init();
  memory_init(name, memory);
  if (0 == read_text_header_items(inp, &t1, &t2, &nor, &t3, in, name)) {
    fclose(inp);
    exit(1);
  }
  elts_per_word = bits_in_word / nob;
  /* Now compute the number of pieces for both rows and columns */
  /* And produce the description file */
  /* Align split to word boundary */
  split = ((split + elts_per_word - 1) / elts_per_word) * elts_per_word;
  row_pieces = (nor + split - 1) / split;
  outputs = my_malloc(row_pieces * sizeof(FILE *));
  output_map(name, out, row_pieces, row_pieces, &names);
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
    u32 j;
    int k;
    row_init(row, len);
    k = fscanf(inp, "%u", &j);
    NOT_USED(k);
    assert(j >= 1);
    put_element_to_clean_row_with_params(nob, j - 1, elts_per_word, row, 1);
    if (0 == ex_row_put(i, nor, nor, out, names, split, row, outputs)) {
      fprintf(stderr, "%s: cannot write output row %u\n", name, i);
      fclose(inp);
      exit(1);
    }
  }
  fclose(inp);
  return 0;
}
