/*
 * $Id: conj.c,v 1.10 2005/07/24 09:32:45 jon Exp $
 *
 * Function to compute algebraic conjugate of a matrix, from file
 *
 */

#include "conj.h"
#include <stdio.h>
#include <assert.h>
#include <errno.h>
#include "elements.h"
#include "endian.h"
#include "header.h"
#include "memory.h"
#include "primes.h"
#include "read.h"
#include "utils.h"
#include "write.h"

static int cleanup(FILE *inp, FILE *outp)
{
  if (NULL != inp) {
    fclose(inp);
  }
  if (NULL != outp) {
    fclose(outp);
  }
  return 0;
}

int conjugate(const char *m1, const char *m2, u32 power, const char *name)
{
  FILE *inp = NULL;
  FILE *outp = NULL;
  u32 prime_power, prime, index, nob, noc, nor, len, elts_per_word;
  u32 i;
  word mask;
  const header *h;
  word *row;
  prime_ops ops;

  assert(NULL != m1);
  assert(NULL != m2);
  assert(NULL != name);
  if (0 == open_and_read_binary_header(&inp, &h, m1, name)) {
    return 0;
  }
  prime_power = header_get_prime(h);
  if (1 == prime_power) {
    /* This is just a copy */
    if (0 == open_and_write_binary_header(&outp, h, m2, name)) {
      header_free(h);
      fclose(inp);
      return 0;
    }
    copy_rest(outp, inp);
    fclose(inp);
    fclose(outp);
    header_free(h);
    return 0;
  }
  prime = prime_divisor(prime_power);
  nob = header_get_nob(h);
  nor = header_get_nor(h);
  noc = header_get_noc(h);
  len = header_get_len(h);
  if (0 == open_and_write_binary_header(&outp, h, m2, name)) {
    header_free(h);
    fclose(inp);
    return 0;
  }
  header_free(h);
  mask = get_mask_and_elts(nob, &elts_per_word);
  if (memory_rows(len, 1000) < 1) {
    fprintf(stderr, "%s: cannot allocate row for %s, %s, terminating\n", name, m1, m2);
    return cleanup(inp, outp);
  }
  primes_init(prime_power, &ops);
  index = prime_index(prime_power, prime);
  power = power % index;
  row = memory_pointer_offset(0, 0, len);
  (void)int_pow(prime, power, &power); /* Convert from index to power */
  for (i = 0; i < nor; i++) {
    errno = 0;
    if (0 == endian_read_row(inp, row, len)) {
      if ( 0 != errno) {
        perror(name);
      }
      fprintf(stderr, "%s: cannot read row %u from %s, terminating\n", name, i, m1);
      return cleanup(inp, outp);
    }
    if ( 1 != power) {
      u32 j;
      for (j = 0; j < noc; j++) {
        word elt = get_element_from_row_with_params(nob, j, mask, elts_per_word, row);
        elt = (*ops.power)(elt, power);
        put_element_to_row_with_params(nob, j, mask, elts_per_word, row, elt);
      }
    }
    errno = 0;
    if (0 == endian_write_row(outp, row, len)) {
      if ( 0 != errno) {
        perror(name);
      }
      fprintf(stderr, "%s: cannot write row %u to %s, terminating\n", name, i, m2);
      return cleanup(inp, outp);
    }
  }
  (void)cleanup(inp, outp);
  return 1;
}
