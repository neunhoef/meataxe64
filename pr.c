/*
 * $Id: pr.c,v 1.6 2001/10/16 22:55:53 jon Exp $
 *
 * Print a matrix
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

/* Purely for formatting purposes */
#define BITS_PER_ROW 80

static const char *name = "zpr";

static void pr_usage(void)
{
  fprintf(stderr, "%s: usage: %s <in_file>\n", name, name);
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
  memory_init(name, 0);
  if (2 != argc) {
    pr_usage();
    exit(1);
  }
  in = argv[1];
  inp = fopen(in, "rb");
  if (NULL == inp) {
    fprintf(stderr, "%s: cannot open %s, terminating\n", name, in);
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
    fprintf(stderr, "%s: cannot write text header, terminating\n", name);
    fclose(inp);
    exit(1);
  }
  if (memory_rows(len, 1000) < 1) {
    fprintf(stderr, "%s: cannot allocate row for %s, terminating\n", name, in);
    fclose(inp);
    exit(1);
  }
  row = memory_pointer_offset(0, 0, len);
  if (0 == primes_init(prime, &prime_operations)) {
    fprintf(stderr, "%s: cannot initialise prime operations, terminating\n", name);
    fclose(inp);
    exit(1);
  }
  for (i = 0; i < nor; i++) {
    unsigned int m = 0;
    if (0 == endian_read_row(inp, row, len)) {
      fprintf(stderr, "%s: cannot read row %d from %s, terminating\n", name, i, in);
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
        fprintf(stderr, "%s: cannot convert %d with prime %d from %s, terminating\n", name, e, prime, in);
        fclose(inp);
        exit(1);
      }
      (void)sprintf(buf, "%0d", e);
      k = strlen(buf);
      if (k > nod) {
        /* Some precision will be lost */
        /* This shouldn't happen */
        fprintf(stderr, "%s: cannot print %d to precision %d without loss of data, terminating\n", name, e, nod);
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
  memory_dispose();
  return 0;
}
