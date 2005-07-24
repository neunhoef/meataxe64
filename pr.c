/*
 * $Id: pr.c,v 1.19 2005/07/24 11:31:35 jon Exp $
 *
 * Print a matrix
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
#include "utils.h"
#include "write.h"

/* Purely for formatting purposes */
#define BITS_PER_ROW 80

static const char *name = "zpr";

static void pr_usage(void)
{
  fprintf(stderr, "%s: usage: %s %s <in_file>\n", name, name, parse_usage());
}

int main(int argc, const char * const argv[])
{
  const char *in;
  FILE *inp;
  u32 prime, nod, noc, nor, nob, len;
  u32 i, j, elts_per_word;
  word mask;
  const header *h;
  word *row;
  prime_ops prime_operations;

  endian_init();
  argv = parse_line(argc, argv, &argc);
  if (2 != argc) {
    pr_usage();
    exit(1);
  }
  in = argv[1];
  memory_init(name, memory);
  if (0 == open_and_read_binary_header(&inp, &h, in, name)) {
    exit(1);
  }
  prime = header_get_prime(h);
  nob = header_get_nob(h);
  nod = header_get_nod(h);
  nor = header_get_nor(h);
  noc = header_get_noc(h);
  len = header_get_len(h);
  if (0 == write_text_header(stdout, h, name)) {
    fprintf(stderr, "%s: cannot write text header, terminating\n", name);
    fclose(inp);
    exit(1);
  }
  header_free(h);
  if (1 == prime) {
    /* A permutation or map */
    for (i = 0; i < nor; i++) {
      word e;
      errno = 0;
      if (1 != endian_read_word(&e, inp)) {
        if ( 0 != errno) {
          perror(name);
        }
        fprintf(stderr, "%s: failed to read entry %u from %s, terminating\n", name, i, in);
        fclose(inp);
        exit(1);
      }
      printf("%12llu\n", (unsigned long long)(e + 1));
    }
  } else {
    mask = get_mask_and_elts(nob, &elts_per_word);
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
      u32 m = 0;
      errno = 0;
      if (0 == endian_read_row(inp, row, len)) {
        if ( 0 != errno) {
          perror(name);
        }
        fprintf(stderr, "%s: cannot read row %u from %s, terminating\n", name, i, in);
        fclose(inp);
        exit(1);
      }
      for (j = 0; j < noc; j++) {
        word e;
        char buf[12];
        u32 k;
        m = j; /* To survive the loop */
        e = get_element_from_row_with_params(nob, j, mask, elts_per_word, row);
        if (0 == (*prime_operations.decimal_rep)(&e)) {
          fprintf(stderr, "%s: cannot convert %llu with prime %u from %s, terminating\n", name, (unsigned long long)e, prime, in);
          fclose(inp);
          exit(1);
        }
        (void)sprintf(buf, "%0u", (u32)e);
        k = strlen(buf);
        if (k > nod) {
          /* Some precision will be lost */
          /* This shouldn't happen */
          fprintf(stderr, "%s: cannot print %llu to precision %u without loss of data, terminating\n", name, (unsigned long long)e, nod);
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
  }
  return 0;
}
