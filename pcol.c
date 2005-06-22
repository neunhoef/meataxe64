/*
 * $Id: pcol.c,v 1.1 2005/06/22 21:52:53 jon Exp $
 *
 * Permuation condense one group element wrt non trivial linear representation
 *
 */

#include "pcol.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <errno.h>
#include "endian.h"
#include "elements.h"
#include "header.h"
#include "maps.h"
#include "memory.h"
#include "orbit.h"
#include "primes.h"
#include "read.h"
#include "rows.h"
#include "utils.h"
#include "write.h"

int pcondense_lambda(const char *in1, const char *in2, const char *in3,
                     const char *out, const char *name)
{
  FILE *orbf, *orblf, *genf, *outf;
  const header *orbh, *orblh, *genh, *outh;
  u32 characteristic, prime, nor, nor_o, len, nob, i, elts_per_word;
  word *map, *row, *word_row;
  u32 *orbit_numbers; /* Tell us which orbit each point lives in */
  u32 *orbits_to_counted, *counted_to_orbits; /* Map between all orbits and those that count */
  u32 *positions;
  prime_ops operations;
  orbit_set *orbits, *orbits_lambda;
  assert(NULL != in1);
  assert(NULL != in2);
  assert(NULL != in3);
  assert(NULL != out);
  assert(NULL != name);
  if (0 == open_and_read_binary_header(&orbf, &orbh, in1, name)) {
    exit(1);
  }
  if (0 == open_and_read_binary_header(&orblf, &orblh, in2, name)) {
    fclose(orbf);
    header_free(orbh);
    exit(1);
  }
  prime = header_get_prime(orblh);
  if (0 == is_a_prime_power(prime)) {
    fprintf(stderr, "%s: field order from %s must be a prime power, terminating\n", in2, name);
    fclose(orbf);
    fclose(orblf);
    header_free(orbh);
    header_free(orblh);
    exit(1);
  }
  if (0 == open_and_read_binary_header(&genf, &genh, in3, name)) {
    fclose(orbf);
    fclose(orblf);
    header_free(orbh);
    header_free(orblh);
    exit(1);
  }
  characteristic = prime_divisor(prime);
  primes_init(prime, &operations);
  nor = header_get_nor(orbh);
  if (1 != header_get_prime(orbh) ||
      1 != header_get_prime(genh) ||
      nor != header_get_noc(orbh) ||
      nor != header_get_nor(orblh) ||
      nor != header_get_noc(orblh) ||
      nor != header_get_nor(genh) ||
      nor != header_get_noc(genh)) {
    fprintf(stderr, "%s: parameter incompatibility between %s and %s, terminating\n", name, in1, in2);
    fclose(orbf);
    fclose(orblf);
    fclose(genf);
    header_free(orbh);
    header_free(orblh);
    header_free(genh);
    exit(1);
  }
  header_free(genh);
  if (0 == read_orbits(orbf, nor, &orbits, in1, name)) {
    fprintf(stderr, "%s: cannot read orbit file %s, terminating\n", name, in1);
    fclose(orbf);
    fclose(orblf);
    fclose(genf);
    header_free(orbh);
    header_free(orblh);
    exit(1);
  }
  fclose(orbf);
  header_free(orbh);
  if (0 == read_orbits(orblf, nor, &orbits_lambda, in2, name)) {
    fprintf(stderr, "%s: cannot read orbit file %s, terminating\n", name, in2);
    fclose(orblf);
    fclose(genf);
    header_free(orblh);
    exit(1);
  }
  fclose(orblf);
  header_free(orblh);
  map = malloc_map(nor);
  if (0 == read_map(genf, nor, map, in3, name)) {
    fclose(genf);
    exit(1);
  }
  fclose(genf);
  orbits_to_counted = my_malloc(orbits->size * sizeof(*orbits_to_counted));
  counted_to_orbits = my_malloc(orbits->size * sizeof(*counted_to_orbits));
  memset(orbits_to_counted, 0xff, orbits->size * sizeof(*orbits_to_counted));
  memset(counted_to_orbits, 0, orbits->size * sizeof(*counted_to_orbits));
  nor_o = 0;
  for (i = 0; i < orbits->size; i++) {
    orbit *o = orbits_lambda->orbits + i;
    if (0 != o->values[0]) {
      orbits_to_counted[i] = nor_o; /* Found an orbit which contributes */
      counted_to_orbits[nor_o] = i; /* Remember which orbit overall */
      nor_o++; /* Increment for the orbits that will count */
    }
  }
  nob = bits_of(prime);
  outh = header_create(prime, nob, digits_of(prime), nor_o, nor_o);
  len = header_get_len(outh);
  if (memory_rows(len, 1000) < 1) {
    fprintf(stderr, "%s: cannot get enough memory for one row, terminating\n", name);
    free(orbits_to_counted);
    free(counted_to_orbits);
    exit(1);
  }
  row = memory_pointer(0);
  word_row = my_malloc(nor_o * sizeof(*word_row));
  orbit_numbers = my_malloc(nor * sizeof(u32));
  if (0 == open_and_write_binary_header(&outf, outh, out, name)) {
    free(orbits_to_counted);
    free(counted_to_orbits);
    free(orbit_numbers);
    exit(1);
  }
  positions = my_malloc(nor * sizeof(*positions));
  /* Work out the points to orbits map, and the positions within orbits */
  memset(orbit_numbers, 0xff, nor * sizeof(u32));
  for (i = 0; i < orbits->size; i++) {
    orbit *orb = orbits->orbits + i;
    u32 j;
    for (j = 0; j < orb->size; j++) {
      assert(orb->values[j] < nor);
      orbit_numbers[orb->values[j]] = i;
      positions[orb->values[j]] = j;
    }
  }
  (void)get_mask_and_elts(nob, &elts_per_word);
  /* Do the real work */
  for (i = 0; i < nor_o; i++) {
    u32 j, k;
    u32 s = counted_to_orbits[i];
    orbit *orb = orbits->orbits + s;
    row_init(row, len);
    for (j = 0; j < nor_o; j++) {
      word_row[j] = 0; /* Initialise the output */
    }
    for (j = 0; j < orb->size; j++) {
      u32 m, n, r;
      word lambda = orbits_lambda->orbits[s].values[j]; /* The character value for this point */
      word mu;
      assert(0 != lambda);
      if (1 != lambda) {
        lambda = operations.invert(lambda);
      }
      k = map[orb->values[j]]; /* Where does this element go */
      assert(k < nor);
      m = orbit_numbers[k];
      assert(m < orbits->size);
      n = orbits_to_counted[m];
      if (0xffffffff != n) {
        assert(n < nor_o);
        r = positions[k]; /* The position in the orbit of this point */
        assert(r < orbits->orbits[m].size);
        mu = orbits_lambda->orbits[m].values[r];
        mu = operations.mul(mu, lambda);
        word_row[n] = operations.add(word_row[n], mu);
      }
    }
    /* Now divide each sum by the corresponding orbit size */
    for (j = 0; j < nor_o; j++) {
      u32 s = counted_to_orbits[j];
      word l = word_row[j]; /* Get the sum produced by the previous stage */
      if ( 0 != l) {
        k = (orbits->orbits[s]).size;
        if (0 == k % characteristic) {
          fprintf(stderr, "%s: orbit size %d is divisible by field characteristic %d, terminating\n", name, k, characteristic);
          fclose(outf);
          header_free(outh);
          orbit_set_free(orbits);
          orbit_set_free(orbits_lambda);
          map_free(map);
          free(word_row);
          free(orbit_numbers);
          free(orbits_to_counted);
          free(counted_to_orbits);
          free(positions);
          exit(1);
        }
        k = k % characteristic;
        assert(0 != k);
        if (1 != k) {
          k = operations.invert(k);
        }
        l = operations.mul(l, k);
        put_element_to_clean_row_with_params(nob, j, elts_per_word, row, l);
      }
    }
    errno = 0;
    if (0 == endian_write_row(outf, row, len)){
      if ( 0 != errno) {
        perror(name);
      }
      fprintf(stderr, "%s: failed to output row %d to %s, terminating\n", name, i, out);
      fclose(outf);
      header_free(outh);
      orbit_set_free(orbits);
      orbit_set_free(orbits_lambda);
      map_free(map);
      free(word_row);
      free(orbit_numbers);
      free(orbits_to_counted);
      free(counted_to_orbits);
      free(positions);
      exit(1);
    }
  }
  fclose(outf);
  header_free(outh);
  orbit_set_free(orbits);
  orbit_set_free(orbits_lambda);
  map_free(map);
  free(word_row);
  free(orbit_numbers);
  free(orbits_to_counted);
  free(counted_to_orbits);
  free(positions);
  return 1;
}
