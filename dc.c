/*
 * $Id: dc.c,v 1.1 2005/06/22 21:52:53 jon Exp $
 *
 * Function to compute direct complement
 * Uses the computed map, rather than clean/echelise
 * Works entirely in RAM
 *
 */

#include "dc.h"
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

static void cleanup(FILE *f1, FILE *f2)
{
  if (NULL != f1)
    fclose(f1);
  if (NULL != f2)
    fclose(f2);
}

void direct_complement(const char *range, const char *out, const char *name)
{
  FILE *inp = NULL, *outp = NULL;
  const header *h_in, *h_out;
  u32 prime, nob, noc, nor, nor_o, noc_o, len, i, j, elts_per_word;
  word *row_in, *row_out;
  word mask;
  int *map, *map1;
  assert(NULL != range);
  assert(NULL != out);
  assert(NULL != name);
  if (0 == open_and_read_binary_header(&inp, &h_in, range, name)) {
    exit(1);
  }
  prime = header_get_prime(h_in);
  if (1 == prime) {
    fprintf(stderr, "%s: cannot handle maps, terminating\n", name);
    cleanup(inp, NULL);
    exit(1);
  }
  nob = header_get_nob(h_in);
  nor = header_get_nor(h_in);
  noc = header_get_noc(h_in);
  len = header_get_len(h_in);
  if (nor >= noc) {
    fprintf(stderr, "%s: too many rows in %s, terminating\n",
            name, range);
    cleanup(inp, NULL);
    exit(1);
  }
  noc_o = noc;
  nor_o = noc - nor;
  if (memory_rows(len, 1000) < 2) {
    fprintf(stderr, "%s: cannot allocate space for two rows from %s, terminating\n",
            name, range);
    cleanup(inp, NULL);
    exit(1);
  }
  row_in = memory_pointer(0);
  row_out = memory_pointer(500);
  h_out = header_create(prime, nob, header_get_nod(h_in), noc_o, nor_o);
  header_free(h_in);
  if (0 == open_and_write_binary_header(&outp, h_out, out, name)) {
    cleanup(inp, outp);
    exit(1);
  }
  header_free(h_out);
  map = my_malloc(nor * sizeof(*map));
  if (0 == subspace_map(inp, map, nor, len, nob, row_in, range, name)) {
    cleanup(inp, outp);
    free(map);
    exit(1);
  }
  /* inp no longer necessary */
  fclose(inp);
  mask = get_mask_and_elts(nob, &elts_per_word);
  map1 = my_malloc(noc * sizeof(*map1));
  for (i = 0; i < noc; i++) {
    map1[i] = -1;
  }
  for (i = 0; i < nor; i++) {
    int k = map[i];
    assert(k >= 0 && (u32)k < noc);
    map1[k] = 1;
  }
  for (i = 0; i < noc; i++) {
    if (map1[i] < 0) {
      /* Significant row */
      /* Clean */
      row_init(row_out, len);
      /* Set bit i */
      put_element_to_clean_row_with_params(nob, i, elts_per_word, row_out, 1);
      /* Write it */
      errno = 0;
      if (0 == endian_write_row(outp, row_out, len)) {
        if ( 0 != errno) {
          perror(name);
        }
        fprintf(stderr, "%s: cannot write row to %s, terminating\n", name, out);
        cleanup(NULL, outp);
        free(map);
        free(map1);
        exit(1);
      }
      j++;
    }
  }
  fclose(outp);
  free(map);
  free(map1);
}
