/*
 * $Id: eid.c,v 1.7 2004/01/31 13:24:51 jon Exp $
 *
 * Generate exploded identity matrix
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "exrows.h"
#include "elements.h"
#include "endian.h"
#include "files.h"
#include "map.h"
#include "memory.h"
#include "parse.h"
#include "primes.h"
#include "rows.h"
#include "utils.h"

static const char *name = "eid";

static void id_usage(void)
{
  fprintf(stderr, "%s: usage: %s %s <field order> <nor> <noc> <split> <out_dir>\n", name, name, parse_usage());
}

int main(int argc, const char * const argv[])
{
  const char *out;
  unsigned int prime, nob, noc, nor, len;
  unsigned int split, elts_in_word;
  unsigned int cols, rows;
  unsigned int i;
  unsigned int *row;
  const char **names;
  char *ptr;
  FILE **outputs;

  argv = parse_line(argc, argv, &argc);
  if (6 != argc) {
    id_usage();
    exit(1);
  }
  out = argv[4];
  prime = strtoul(argv[1], NULL, 0);
  if (0 == is_a_prime_power(prime)) {
    fprintf(stderr, "%s: non prime %d\n", name, prime);
    exit(1);
  }
  nor = strtoul(argv[2], NULL, 0);
  noc = strtoul(argv[3], NULL, 0);
  split = strtoul(argv[4], &ptr, 0);
  nob = bits_of(prime);
  elts_in_word = bits_in_unsigned_int / nob;
  /* Align split to word boundary */
  split = ((split + elts_in_word - 1) / elts_in_word) * elts_in_word;
  cols = (noc + split - 1) / split;
  rows = (nor + split - 1) / split;
  outputs = my_malloc(cols * sizeof(FILE *));
  output_map(name, argv[5], cols, rows, &names);
  endian_init();
  memory_init(name, memory);
  len = (noc + elts_in_word - 1) / elts_in_word;
  if (memory_rows(len, 1000) < 1) {
    fprintf(stderr, "%s: cannot create output row\n", name);
    exit(1);
  }
  row = memory_pointer_offset(0, 0, len);
  for (i = 0; i < nor; i++) {
    row_init(row, len);
    if (i < noc) {
      put_element_to_row(nob, i, row, 1);
    }
    /* Write the row */
    ex_row_put(i, noc, nor, argv[5], names, split, row, outputs);
  }
  memory_dispose();
  return 0;
}
