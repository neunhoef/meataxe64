/*
 * $Id: conj.c,v 1.3 2002/03/09 19:18:02 jon Exp $
 *
 * Function to compute algebraic conjugate of a matrix, from file
 *
 */

#include "conj.h"
#include <stdio.h>
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

int conjugate(const char *m1, const char *m2, unsigned int power, const char *name)
{
  FILE *inp = NULL;
  FILE *outp = NULL;
  unsigned int prime_power, prime, index, nob, noc, nor, len;
  unsigned int i;
  const header *h;
  unsigned int *row;
  prime_ops ops;
  if (0 == open_and_read_binary_header(&inp, &h, m1, name)) {
    if (NULL != inp) {
      fclose(inp);
      header_free(h);
    }
    return 0;
  }
  prime_power = header_get_prime(h);
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
  if (memory_rows(len, 1000) < 1) {
    fprintf(stderr, "%s cannot allocate row for %s, %s, terminating\n", name, m1, m2);
    return cleanup(inp, outp);
  }
  primes_init(prime_power, &ops);
  index = prime_index(prime_power, prime);
  power = power % index;
  row = memory_pointer_offset(0, 0, len);
  (void)int_pow(prime, power, &power); /* Convert from index to power */
  for (i = 0; i < nor; i++) {
    if (0 == endian_read_row(inp, row, len)) {
      fprintf(stderr, "%s cannot read row %d from %s, terminating\n", name, i, m1);
      return cleanup(inp, outp);
    }
    if ( 1 != power) {
      unsigned int j;
      for (j = 0; j < noc; j++) {
        unsigned int elt = get_element_from_row(nob, j, row);
        elt = (*ops.power)(elt, power);
        put_element_to_row(nob, j, row, elt);
      }
    }
    if (0 == endian_write_row(outp, row, len)) {
      fprintf(stderr, "%s cannot write row %d to %s, terminating\n", name, i, m2);
      return cleanup(inp, outp);
    }
  }
  (void)cleanup(inp, outp);
  return 1;
}
