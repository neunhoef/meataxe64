/*
 * $Id: pr.c,v 1.4 2001/09/12 23:13:04 jon Exp $
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
#include "rows.h"

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
  unsigned int prime, nod, noc, nor, nob, len;
  unsigned int i, j;
  const header *h;
  unsigned int *row;
  prime_ops prime_operations;

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
  nob = header_get_nob(h);
  nod = header_get_nod(h);
  nor = header_get_nor(h);
  noc = header_get_noc(h);
  len = header_get_len(h);
  if (0 == write_text_header(stdout, h)) {
    fprintf(stderr, "pr: cannot write text header, terminating\n");
    fclose(inp);
    exit(1);
  }
  if (0 == row_malloc(len, &row)) {
    fprintf(stderr, "pr: cannot allocate row for %s, terminating\n", in);
    fclose(inp);
    exit(1);
  }
  if (0 == primes_init(prime, &prime_operations)) {
    fprintf(stderr, "pr: cannot initialise prime operations, terminating\n");
    fclose(inp);
    exit(1);
  }
  for (i = 0; i < nor; i++) {
    unsigned int m = 0;
    if (0 == endian_read_row(inp, row, len)) {
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
      if (0 == (*prime_operations.decimal_rep)(&e)) {
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
  return 0;
}
