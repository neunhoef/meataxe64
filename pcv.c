/*
 * $Id: pcv.c,v 1.1 2002/06/25 10:30:12 jon Exp $
 *
 * Function to lift vectors from a quotient space
 *
 */

#include "pcv.h"
#include "elements.h"
#include "endian.h"
#include "header.h"
#include "memory.h"
#include "orbit.h"
#include "primes.h"
#include "read.h"
#include "rows.h"
#include "utils.h"
#include "write.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

static void cleanup(FILE *f1, FILE *f2, FILE *f3)
{
  if (NULL != f1)
    fclose(f1);
  if (NULL != f2)
    fclose(f2);
  if (NULL != f3)
    fclose(f3);
}

void lift(const char *range, const char *vectors,
          const char *out, const char *name)
{
  FILE *inp1 = NULL, *inp2 = NULL, *outp = NULL;
  const header *h_in1, *h_in2, *h_out;
  unsigned int prime, nob, noc, nor, nor_o, noc_o, len, len_o, i, j, elt;
  unsigned int *row_in, *row_out;
  orbit_set *orbits;
  assert(NULL != range);
  assert(NULL != vectors);
  assert(NULL != out);
  assert(NULL != name);
  if (0 == open_and_read_binary_header(&inp1, &h_in1, range, name) ||
      0 == open_and_read_binary_header(&inp2, &h_in2, vectors, name)) {
    fprintf(stderr, "%s: failed to open or read header from one of %s, %s, terminating\n",
            name, range, vectors);
    cleanup(inp1, inp2, NULL);
    exit(1);
  }
  if (1 != header_get_prime(h_in1)) {
    fprintf(stderr, "%s: %s is not an orbit set\n", name, range);
    cleanup(inp1, inp2, NULL);
    exit(1);
  }
  prime = header_get_prime(h_in2);
  noc_o = header_get_noc(h_in1);
  nob = header_get_nob(h_in2);
  nor = header_get_nor(h_in2);
  noc = header_get_noc(h_in2);
  len = header_get_len(h_in2);
  /* One entry for each incoming and for each subspace row */
  nor_o = header_get_nor(h_in2);
  if (header_get_nor(h_in1) != noc_o) {
    fprintf(stderr, "%s: incompatible parameters for %s, %s, terminating\n",
            name, range, vectors);
    cleanup(inp1, inp2, NULL);
    exit(1);
  }
  h_out = header_create(prime, nob, header_get_nod(h_in2), noc_o, nor_o);
  header_free(h_in1);
  header_free(h_in2);
  len_o = header_get_len(h_out);
  if (memory_rows(len_o, 1000) < 2) {
    fprintf(stderr, "%s: cannot allocate space for two rows for %s, %s, terminating\n",
            name, range, vectors);
    cleanup(inp1, inp2, NULL);
    exit(1);
  }
  row_in = memory_pointer(0);
  row_out = memory_pointer(500);
  if (0 == open_and_write_binary_header(&outp, h_out, out, name)) {
    cleanup(inp1, inp2, outp);
    exit(1);
  }
  header_free(h_out);
  if (0 == read_orbits(inp1, noc_o, &orbits, range, name)) {
    cleanup(inp1, inp2, outp);
    exit(1);
  }
  /* inp1 no longer necessary */
  fclose(inp1);
  /* Do some more compatibility checks */
  if (noc != orbits->size) {
      fprintf(stderr, "%s: number of orbits of %s and row length of %s disagree, terminating\n", name, range, vectors);
      cleanup(NULL, inp2, outp);
      exit(1);
  }
  /* Now step through vectors computing lifted representation */
  for (i = 0; i < nor_o; i += 1) {
    /* Initialise output row */
    row_init(row_out, len_o);
    /* Read row of vectors */
    if (0 == endian_read_row(inp2, row_in, len)) {
      fprintf(stderr, "%s: cannot read row from %s, terminating\n", name, vectors);
      cleanup(NULL, inp2, outp);
      exit(1);
    }
    /* Set the elements */
    for (j = 0; j < noc; j++) {
      elt = get_element_from_row(nob, j, row_in);
      if (0 != elt) {
        /* Output elt at each orbit element position */
        unsigned int k, l;
        orbit *orb = orbits->orbits + j;
        for (k = 0; k < orb->size; k++) {
          l = orb->values[k];
          assert(l < noc_o);
          put_element_to_row(nob, l, row_out, elt);
        }
      }
    }
    if (0 == endian_write_row(outp, row_out, len_o)) {
      fprintf(stderr, "%s: cannot write row to %s, terminating\n", name, out);
      cleanup(NULL, inp2, outp);
      exit(1);
    }
  }
  fclose(inp2);
  fclose(outp);
}
