/*
 * $Id: ipp.c,v 1.4 2001/11/25 12:44:33 jon Exp $
 *
 * Read a permutation into a matrix
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <limits.h>
#include "elements.h"
#include "endian.h"
#include "header.h"
#include "memory.h"
#include "primes.h"
#include "read.h"
#include "rows.h"
#include "utils.h"
#include "write.h"

static const char *name = "zip";

static void zip_usage(void)
{
  fprintf(stderr, "%s: usage: %s <in_file> <out_file> <prime>\n", name, name);
}

int main(int argc, const char * const argv[])
{
  const char *in;
  const char *out;
  FILE *inp;
  FILE *outp;
  unsigned int prime, nob, nod, nor, len;
  unsigned int i;
  unsigned int t1, t2, t3;
  const header *h;
  unsigned int *row;

  if (4 != argc) {
    zip_usage();
    exit(1);
  }
  in = argv[1];
  out = argv[2];
  prime = strtoul(argv[3], NULL, 0);
  if (0 == is_a_prime_power(prime)) {
    fprintf(stderr, "%s: %d is not a prime power, terminating\n", name, prime);
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
    exit(1);
  }
  h = header_create(prime, nob, nod, nor, nor);
  len = header_get_len(h);
  if (memory_rows(len, 1000) < 1) {
    fprintf(stderr, "%s: cannot create output row\n", name);
    fclose(inp);
    exit(1);
  }
  row = memory_pointer_offset(0, 0, len);
  assert(NULL != row);
  if (0 == open_and_write_binary_header(&outp, h, out, name)) {
    fclose(inp);
    exit(1);
  }
  for (i = 0; i < nor; i++) {
    unsigned int j;
    row_init(row, len);
    j = getin(inp, 7);
    assert(j >= 1);
    put_element_to_row(nob, j - 1, row, 1);
    if (0 == endian_write_row(outp, row, len)) {
      fprintf(stderr, "%s: write output row to %s\n", name, out);
      fclose(inp);
      fclose(outp);
      exit(1);
    }
  }
  fclose(inp);
  fclose(outp);
  return 0;
}
