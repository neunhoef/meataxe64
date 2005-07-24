/*
 * $Id: ipp.c,v 1.15 2005/07/24 09:32:45 jon Exp $
 *
 * Read a permutation into a matrix
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <limits.h>
#include <errno.h>
#include "elements.h"
#include "endian.h"
#include "header.h"
#include "memory.h"
#include "parse.h"
#include "primes.h"
#include "read.h"
#include "rows.h"
#include "utils.h"
#include "write.h"

static const char *name = "zip";

static void zip_usage(void)
{
  fprintf(stderr, "%s: usage: %s %s <in_file> <out_file> <field order>\n", name, name, parse_usage());
}

int main(int argc, const char * const argv[])
{
  const char *in;
  const char *out;
  FILE *inp;
  FILE *outp;
  u32 prime, nob, nod, nor, len;
  u32 i;
  u32 t1, t2, t3;
  const header *h;
  word *row;

  argv = parse_line(argc, argv, &argc);
  if (4 != argc) {
    zip_usage();
    exit(1);
  }
  in = argv[1];
  out = argv[2];
  prime = strtoul(argv[3], NULL, 0);
  if (0 == is_a_prime_power(prime)) {
    fprintf(stderr, "%s: %u is not a prime power, terminating\n", name, prime);
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
  header_free(h);
  for (i = 0; i < nor; i++) {
    u32 j;
    row_init(row, len);
    fscanf(inp, "%u", &j);
    if (0 == j || j > nor) {
      fprintf(stderr, "%s: %u (out of range 1 - %u) found as permutation image, terminating\n", name, j, nor);
      exit(1);
    }
    put_element_to_row(nob, j - 1, row, 1);
    errno = 0;
    if (0 == endian_write_row(outp, row, len)) {
      if ( 0 != errno) {
        perror(name);
      }
      fprintf(stderr, "%s: cannot write output row %u to %s\n", name, i, out);
      fclose(inp);
      fclose(outp);
      exit(1);
    }
  }
  fclose(inp);
  fclose(outp);
  return 0;
}
