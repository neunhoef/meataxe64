/*
 * $Id: ss.c,v 1.14 2002/10/13 16:38:07 jon Exp $
 *
 * Function to compute subspace representation
 * Uses the computed map, rather than clean/echelise
 * Works entirely in RAM
 *
 */

#include "ss.h"
#include "elements.h"
#include "endian.h"
#include "header.h"
#include "memory.h"
#include "primes.h"
#include "read.h"
#include "rows.h"
#include "ss_map.h"
#include "utils.h"
#include "write.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

static void cleanup(FILE *f1, FILE *f2, FILE *f3)
{
  if (NULL != f1)
    fclose(f1);
  if (NULL != f2)
    fclose(f2);
  if (NULL != f3)
    fclose(f3);
}

void subspace(const char *range, const char *image,
              const char *out, const char *name)
{
  FILE *inp1 = NULL, *inp2 = NULL, *outp = NULL;
  const header *h_in1, *h_in2, *h_out;
  unsigned int prime, nob, noc, nor, nor_o, noc_o, len, len_e, i, j, elt, mask, elts_per_word;
  unsigned int *row_in, *row_out;
  int *map;
  assert(NULL != range);
  assert(NULL != image);
  assert(NULL != out);
  assert(NULL != name);
  if (0 == open_and_read_binary_header(&inp1, &h_in1, range, name) ||
      0 == open_and_read_binary_header(&inp2, &h_in2, image, name)) {
    cleanup(inp1, inp2, NULL);
    exit(1);
  }
  prime = header_get_prime(h_in1);
  if (1 == prime) {
    fprintf(stderr, "%s: cannot handle maps, terminating\n", name);
    cleanup(inp1, inp2, NULL);
    exit(1);
  }
  nob = header_get_nob(h_in1);
  nor = header_get_nor(h_in1);
  noc = header_get_noc(h_in1);
  len = header_get_len(h_in1);
  noc_o = nor;
  nor_o = header_get_nor(h_in2);
  if (noc != header_get_noc(h_in2) ||
      prime != header_get_prime(h_in2) ||
      nob != header_get_nob(h_in2)) {
    fprintf(stderr, "%s: incompatible parameters for %s, %s, terminating\n",
            name, range, image);
    cleanup(inp1, inp2, NULL);
    exit(1);
  }
  if (nor > noc) {
    fprintf(stderr, "%s: too many rows in %s, %s, terminating\n",
            name, range, image);
    cleanup(inp1, inp2, NULL);
    exit(1);
  }
  if (memory_rows(len, 1000) < 2) {
    fprintf(stderr, "%s: cannot allocate space for two rows from %s, terminating\n",
            name, range);
    cleanup(inp1, inp2, NULL);
    exit(1);
  }
  row_in = memory_pointer(0);
  row_out = memory_pointer(500);
  h_out = header_create(prime, nob, header_get_nod(h_in1), noc_o, nor_o);
  len_e = header_get_len(h_out);
  assert(header_get_len(h_in2) == len);
  assert(len >= len_e);
  header_free(h_in1);
  header_free(h_in2);
  if (0 == open_and_write_binary_header(&outp, h_out, out, name)) {
    cleanup(inp1, inp2, outp);
    exit(1);
  }
  header_free(h_out);
  map = my_malloc(nor * sizeof(int));
  if (0 == subspace_map(inp1, map, nor, len, nob, row_in, range, name)) {
    cleanup(inp1, inp2, outp);
    exit(1);
  }
  /* inp1 no longer necessary */
  fclose(inp1);
  mask = get_mask_and_elts(nob, &elts_per_word);
  /* Now step through image computing subspace representation */
  for (i = 0; i < nor_o; i += 1) {
    /* Read row of image */
    errno = 0;
    if (0 == endian_read_row(inp2, row_in, len)) {
      if ( 0 != errno) {
        perror(name);
      }
      fprintf(stderr, "%s: cannot read row from %s, terminating\n", name, image);
      cleanup(NULL, inp2, outp);
      exit(1);
    }
    /* Initialise output row */
    row_init(row_out, len_e);
    /* Set the elements */
    for (j = 0; j < nor; j++) {
      assert(0 <= map[j]);
      elt = get_element_from_row_with_params(nob, map[j], mask, elts_per_word, row_in);
      put_element_to_clean_row_with_params(nob, j, elts_per_word, row_out, elt);
    }
    errno = 0;
    if (0 == endian_write_row(outp, row_out, len_e)) {
      if ( 0 != errno) {
        perror(name);
      }
      fprintf(stderr, "%s: cannot write row to %s, terminating\n", name, out);
      cleanup(NULL, inp2, outp);
      exit(1);
    }
  }
  fclose(inp2);
  fclose(outp);
}
