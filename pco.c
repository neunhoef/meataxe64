/*
 * $Id: pco.c,v 1.9 2017/01/10 08:08:47 jon Exp $
 *
 * Permuation condense one group element
 *
 */

#include "pco.h"
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

int pcondense(const char *in1, const char *in2,
              u32 field_order,
              const char *out, const char *name)
{
  FILE *orbf, *genf, *outf;
  const header *orbh, *genh, *outh;
  u32 characteristic, nor, nor_o, len, nob, i, elts_per_word;
  word *map, *row, *int_row;
  u32 *orbit_numbers;
  prime_ops operations;
  orbit_set *orbits;
  assert(NULL != in1);
  assert(NULL != in2);
  assert(NULL != out);
  assert(NULL != name);
  if (0 == is_a_prime_power(field_order)) {
    fprintf(stderr, "%s: field order must be a prime power, terminating\n", name);
    exit(1);
  }
  characteristic = prime_divisor(field_order);
  primes_init(field_order, &operations);
  if (0 == open_and_read_binary_header(&orbf, &orbh, in1, name) ||
      0 == open_and_read_binary_header(&genf, &genh, in2, name)) {
    if (NULL != orbh) {
      header_free(orbh);
      fclose(orbf);
    }
    exit(1);
  }
  nor = header_get_nor(orbh);
  if (1 != header_get_prime(orbh) ||
      1 != header_get_prime(genh) ||
      nor != header_get_noc(orbh) ||
      nor != header_get_nor(genh) ||
      nor != header_get_noc(genh)) {
    fprintf(stderr, "%s: parameter incompatibility between %s and %s, terminating\n", name, in1, in2);
    fclose(orbf);
    fclose(genf);
    exit(1);
  }
  if (0 == read_orbits(orbf, nor, &orbits, in1, name)) {
    fprintf(stderr, "%s: cannot read orbit file %s, terminating\n", name, in1);
    fclose(orbf);
    fclose(genf);
    exit(1);
  }
  fclose(orbf);
  nor_o = orbits->size;
  map = malloc_map(nor);
  if (0 == read_map(genf, nor, map, in2, name)) {
    fclose(genf);
    exit(1);
  }
  fclose(genf);
  nob = bits_of(field_order);
  outh = header_create(field_order, nob, digits_of(field_order), nor_o, nor_o);
  len = header_get_len(outh);
  if (memory_rows(len, 1000) < 1) {
    fprintf(stderr, "%s: cannot get enough memory for one row, terminating\n", name);
    exit(1);
  }
  row = memory_pointer(0);
  int_row = my_malloc(nor_o * sizeof(word));
  orbit_numbers = my_malloc(nor * sizeof(u32));
  if (0 == open_and_write_binary_header(&outf, outh, out, name)) {
    exit(1);
  }
  memset(orbit_numbers, 0xff, nor * sizeof(u32));
  for (i = 0; i < nor_o; i++) {
    orbit *orb = orbits->orbits + i;
    u32 j;
    for (j = 0; j < orb->size; j++) {
      assert(orb->values[j] < nor);
      orbit_numbers[orb->values[j]] = i;
    }
  }
  (void)get_mask_and_elts(nob, &elts_per_word);
  for (i = 0; i < nor_o; i++) {
    orbit *orb = orbits->orbits + i;
    u32 j, k;
    row_init(row, len);
#if 1
    for (j = 0; j < nor_o; j++) {
      int_row[j] = 0;
    }
#else
    memset(int_row, 0, nor_o * sizeof(word));
#endif
    for (j = 0; j < orb->size; j++) {
      k = map[orb->values[j]];
      /* Now find its orbit */
      assert(k < nor);
      assert(orbit_numbers[k] < nor_o);
      int_row[orbit_numbers[k]]++;
    }
    for (j = 0; j < nor_o; j++) {
      word l = int_row[j] % characteristic;
      if ( 0 != l) {
        k = (orbits->orbits[j]).size;
        if (0 == k % characteristic) {
          fprintf(stderr, "%s: orbit size %u is divisible by field characteristic %u, terminating\n", name, k, characteristic);
          fclose(outf);
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
      fprintf(stderr, "%s: failed to output row %u to %s, terminating\n", name, i, out);
      fclose(outf);
      exit(1);
    }
  }
  fclose(outf);
  header_free(outh);
  orbit_set_free(orbits);
  map_free(map);
  free(int_row);
  free(orbit_numbers);
  return 1;
}
