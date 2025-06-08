/*
 * $Id: spanmsp.c,v 1.5 2005/07/24 09:32:45 jon Exp $
 *
 * Function to spin from a span under multiple generators until a proper subspace is found
 *
 */

#include "spanmsp.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "elements.h"
#include "grease.h"
#include "header.h"
#include "mul.h"
#include "primes.h"
#include "parse.h"
#include "read.h"
#include "rows.h"
#include "utils.h"

u32 spanmspin(const char *in, const char *out,
              unsigned int argc, const char * const args[],
              const char *name)
{
  u32 nob, nor, noc, prime, len, scalar_len, i, j;
  const header *h_in;
  FILE *inp;
  word *scalar_row, *seed_row;
  grease_struct grease;
  row_ops row_operations;
  NOT_USED(out);
  NOT_USED(argc);
  NOT_USED(args);
  assert(NULL != in);
  assert(NULL != out);
  assert(0 != argc);
  assert(NULL != args);
  assert(NULL != name);
  if (0 == open_and_read_binary_header(&inp, &h_in, in, name)) {
    return 0;
  }
  nob = header_get_nob(h_in);
  noc = header_get_noc(h_in);
  nor = header_get_nor(h_in);
  prime = header_get_prime(h_in);
  len = header_get_len(h_in);
  if (0 == is_a_prime_power(prime)) {
    fprintf(stderr, "%s: %s is not over a finite field, terminating\n", name, in);
    fclose(inp);
    header_free(h_in);
    return 0;
  }
  if (0 == rows_init(prime, &row_operations)) {
    fprintf(stderr, "%s: cannot initialise row operations for %s, terminating\n", name, in);
    fclose(inp);
    header_free(h_in);
    return 0;
  }
  grease_init(&row_operations, &grease);
  scalar_len = compute_len(nob, nor);
  scalar_row = my_malloc(scalar_len);
  seed_row = my_malloc(len);
  row_init(scalar_row, scalar_len);
  /* Fix len in the following, it is wrong */
  if (0 == grease_level(prime, &grease, len)) {
    fprintf(stderr, "%s: cannot allocate grease space, terminating\n", name);
    exit(2);
  }
  /* Allocate the grease space */
  if (0 == grease_allocate(prime, len, &grease, 900)){
    fprintf(stderr, "%s: unable to allocate grease, terminating\n", name);
    fclose(inp);
    header_free(h_in);
    return 0;
  }
  i = 0;
  for (;;) {
    int broke = 0;
    put_element_to_row(nob, i, scalar_row, 1);
    if (0 == mul_from_store(&scalar_row, &seed_row, inp, 0, nor, len, nob, 1, noc, prime,
                            &grease, verbose, in, name)) {
      fprintf(stderr, "%s: failed to multiply by %s, terminating\n", name, in);
      fclose(inp);
      header_free(h_in);
      return 0;
    }
    /* TODO: call spin from stored vector seed_row */
    /* Now increment */
    for (j = 0; j < i; j++) {
      word elt = get_element_from_row(nob, j, scalar_row);
      word elt1 = (elt + 1) % prime;
      put_element_to_row(nob, j, scalar_row, elt1);
      if (0 != elt1) {
        broke = 1;
        break;
      }
    }
    if (0 == broke) {
      /* Incremented all the way up to i */
      i++;
      row_init(scalar_row, scalar_len);
      if (i >= nor) {
        /* Out of vectors */
        break;
      }
    }
  }
  free(scalar_row);
  free(seed_row);
  return 1;
}
