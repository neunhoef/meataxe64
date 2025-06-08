/*
 * p Permuation condense one group element
 *
 */

#include "ppco.h"
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

int ppcondense(const char *in_orb, const char *in_g, u32 group_order,
               u32 field_order, const char *out, const char *name)
{
  FILE *orbf, *genf, *outf;
  const header *orbh, *genh, *outh;
  u32 characteristic, nor, nor_o, len, nob, i, elts_per_word, tot_orbs;
  word *map, *row, *int_row;
  u32 *orbit_numbers;
  prime_ops operations;
  orbit_set *orbits;
  assert(NULL != in_orb);
  assert(NULL != in_g);
  assert(NULL != out);
  assert(NULL != name);
  if (0 == is_a_prime_power(field_order)) {
    fprintf(stderr, "%s: field order must be a prime power, terminating\n", name);
    exit(1);
  }
  characteristic = prime_divisor(field_order);
  primes_init(field_order, &operations);
  if (0 == open_and_read_binary_header(&orbf, &orbh, in_orb, name) ||
      0 == open_and_read_binary_header(&genf, &genh, in_g, name)) {
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
    fprintf(stderr, "%s: parameter incompatibility between %s and %s, terminating\n", name, in_orb, in_g);
    fclose(orbf);
    fclose(genf);
    exit(1);
  }
  if (0 == read_orbits(orbf, nor, &orbits, in_orb, name)) {
    fprintf(stderr, "%s: cannot read orbit file %s, terminating\n", name, in_orb);
    fclose(orbf);
    fclose(genf);
    exit(1);
  }
  fclose(orbf);
  tot_orbs = orbits->size;
#if 0
  nor_o = tot_orbs;
#else
  /* Here we need the number of orbits of size whose stabilisers are p' groups */
  nor_o = 0;
  for (i = 0; i < tot_orbs; i++) {
    orbit *orb = orbits->orbits + i;
    u32 stab_size = group_order / orb->size;
    /* Consistency check */
    assert(group_order == stab_size * orb->size);
    if (0 != (stab_size % characteristic)) {
      nor_o++;
    }
  }
#endif
  map = malloc_map(nor);
  if (0 == read_map(genf, nor, map, in_g, name)) {
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
  int_row = my_malloc(tot_orbs * sizeof(word));
  orbit_numbers = my_malloc(nor * sizeof(u32));
  if (0 == open_and_write_binary_header(&outf, outh, out, name)) {
    exit(1);
  }
  memset(orbit_numbers, 0xff, nor * sizeof(u32));
  for (i = 0; i < tot_orbs; i++) {
    orbit *orb = orbits->orbits + i;
    u32 j;
    for (j = 0; j < orb->size; j++) {
      assert(orb->values[j] < nor);
      orbit_numbers[orb->values[j]] = i;
    }
  }
  (void)get_mask_and_elts(nob, &elts_per_word);
  for (i = 0; i < tot_orbs; i++) {
    orbit *orb = orbits->orbits + i;
    u32 stab_size = group_order / orb->size;
    if (0 != (stab_size % characteristic)) {
      /* We ignore orbits where the group sum is zero, ie stabiliser is not p' */
      u32 j, k;
      row_init(row, len);
      for (j = 0; j < tot_orbs; j++) {
        int_row[j] = 0;
      }
      for (j = 0; j < orb->size; j++) {
        k = map[orb->values[j]];
        /* Now find its orbit */
        assert(k < nor);
        assert(orbit_numbers[k] < tot_orbs);
        int_row[orbit_numbers[k]]++;
      }
      /*
       * Need care here.
       * We need the sizes of the target orbits
       * These are indexed by the whole orbit set,
       * but we only care about those with p' stabilisers
       */
      k = 0; /* Significant row count */
      for (j = 0; j < tot_orbs; j++) {
        u32 tstab_size = group_order / orbits->orbits[j].size;
        if (0 != (tstab_size % characteristic)) {
          word l = int_row[j] * stab_size % characteristic;
          if (0 != l) {
            u32 m = tstab_size % characteristic;
            if (1 != m) {
              m = operations.invert(m);
            }
            l = operations.mul(l, m);
            put_element_to_clean_row_with_params(nob, k, elts_per_word, row, l);
          }
          k++; /* Significant row*/
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
  }
  fclose(outf);
  header_free(outh);
  orbit_set_free(orbits);
  map_free(map);
  free(int_row);
  free(orbit_numbers);
  return 1;
}
