/*
 * $Id: pr.c,v 1.1 2001/08/30 18:31:45 jon Exp $
 *
 * Print a matrix
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <limits.h>
#include "header.h"
#include "utils.h"
#include "read.h"
#include "write.h"
#include "elements.h"
#include "endian.h"
#include "primes.h"

/* Purely for formatting purposes */
#define BITS_PER_ROW 80

static void pr_usage(void)
{
  fprintf(stderr, "pr: usage: pr <in_file>\n");
}

int main(int argc, const char * const argv[])
{
  const char *in;
  FILE *inp;
  unsigned int prime, nod, noc, nor, nob;
  unsigned int i, j;
  header h;
  unsigned int *row;
  unsigned int row_words;
  unsigned int row_chars;
  unsigned int elts_per_word;

  endian_init();
  if (2 != argc) {
    pr_usage();
    exit(1);
  }
  in = argv[1];
  inp = fopen(in, "rb");
  if (NULL == inp) {
    fprintf(stderr, "pr: cannot open %s, terminating\n", in);
    exit(1);
  }
  if (0 == read_binary_header(inp, &h, in)) {
    fclose(inp);
    exit(1);
  }
  prime = header_get_prime(h);
  nob = header_get_nod(h);
  nod = header_get_nod(h);
  nor = header_get_nor(h);
  noc = header_get_noc(h);
  elts_per_word = bits_in_unsigned_int / nob;
  row_words = (noc + elts_per_word - 1) / elts_per_word;
  row_chars = row_words * sizeof(unsigned int);
  write_text_header(stdout, h);
  row = malloc(row_chars);
  if (NULL == row) {
    fprintf(stderr, "pr: cannot allocate row for %s, terminating\n", in);
    fclose(inp);
    exit(1);
  }
  for (i = 0; i < nor; i++) {
    unsigned int m = 0;

    if (0 == endian_read_row(inp, row, nob, noc)) {
      fprintf(stderr, "pr: cannot read row %d from %s, terminating\n", i, in);
      fclose(inp);
      exit(1);
    }
    for (j = 0; j < noc; j++) {
      unsigned int e;
      char buf[12];
      unsigned int k;
      m = j; /* To survive the loop */
      e = get_element_from_row(nob, j, row);
      if (0 == decimal_rep(&e, prime)) {
        fprintf(stderr, "pr: cannot convert %d with prime %d from %s, terminating\n", e, prime, in);
        fclose(inp);
        exit(1);
      }
      (void)sprintf(buf, "%0d", e);
      k = strlen(buf);
      if (k > nod) {
        /* Some precision will be lost */
        /* This shouldn't happen */
        fprintf(stderr, "pr: cannot print %d to precision %d without loss of data, terminating\n", e, nod);
        fclose(inp);
        exit(1);
      } else if (k < nod) {
        while (k < nod) {
          printf(" ");
          k++;
        }
      } else {
        assert(k == nod);
        /* Nothing else to be done */
      }
      printf(buf);
      if (BITS_PER_ROW - 1 == j % BITS_PER_ROW) {
        printf("\n");
      }
    }
    if (BITS_PER_ROW - 1 != m % BITS_PER_ROW) {
      printf("\n");
    }
  }
  fclose(inp);
  return 1;
}
