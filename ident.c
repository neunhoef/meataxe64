/*
 * $Id: ident.c,v 1.10 2002/06/28 08:39:16 jon Exp $
 *
 * Subroutine to generate identity matrix
 *
 */

#include "ident.h"
#include <stdio.h>
#include <assert.h>
#include <errno.h>
#include "elements.h"
#include "endian.h"
#include "header.h"
#include "maps.h"
#include "memory.h"
#include "primes.h"
#include "rows.h"
#include "utils.h"
#include "write.h"

int ident(unsigned int prime, unsigned int nor, unsigned int noc, unsigned int elt,
          const char *out, const char *name)
{
  unsigned int nob, nod, len;
  unsigned int i;
  unsigned int *row;
  FILE *outp;
  const header *h;

  assert(NULL != out);
  assert(NULL != name);
  if (1 == prime) {
    unsigned int *map;
    if (1 != elt) {
      fprintf(stderr, "%s: cannot scale a permutation\n", name);
      return 0;
    }
    h = header_create(1, 0, 0, noc, nor);
    if (0 == open_and_write_binary_header(&outp, h, out, name)) {
      header_free(h);
      return 0;
    }
    if (nor > noc) {
      fprintf(stderr, "%s: cannot create identity map with more rows than columns\n", name);
      header_free(h);
      return 0;
    }
    map = malloc_map(nor);
    for (i = 0; i < nor; i++) {
      map[i] = i;
    }
    if (0 == write_map(outp, nor, map, name, out)) {
      header_free(h);
      return 0;
    }
    map_free(map);
  } else {
    if (0 == is_a_prime_power(prime)) {
      fprintf(stderr, "%s: non prime %d\n", name, prime);
      return 0;
    }
    if (elt >= prime) {
      fprintf(stderr, "%s: %d is too large for %d\n", name, elt, prime);
      return 0;
    }
    nob = bits_of(prime);
    nod = digits_of(prime);
    h = header_create(prime, nob, nod, noc, nor);
    assert(NULL != h);
    len = header_get_len(h);
    assert(0 != len);
    if (0 == open_and_write_binary_header(&outp, h, out, name)) {
      header_free(h);
      return 0;
    }
    header_free(h);
    if (memory_rows(len, 1000) < 1) {
      fprintf(stderr, "%s: cannot create output row\n", name);
      fclose(outp);
      return 0;
    }
    row = memory_pointer_offset(0, 0, len);
    assert(NULL != row);
    for (i = 0; i < nor; i++) {
      row_init(row, len);
      if (i < noc) {
        put_element_to_row(nob, i, row, elt);
      }
      errno = 0;
      if (0 == endian_write_row(outp, row, len)) {
        if ( 0 != errno) {
          perror(name);
        }
        fprintf(stderr, "%s: cannot write output row to %s\n", name, out);
        fclose(outp);
        return 0;
      }
    }
  }
  fclose(outp);
  return 1;
}
